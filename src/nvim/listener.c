/* Copyright (c) 2019 Maxim Lyapin 
 *  
 *  Permission is hereby granted, free of charge, to any person obtaining a copy 
 *  of this software and associated documentation files (the "Software"), to deal 
 *  in the Software without restriction, including without limitation the rights 
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
 *  copies of the Software, and to permit persons to whom the Software is 
 *  furnished to do so, subject to the following conditions: 
 *   
 *  The above copyright notice and this permission notice shall be included in all 
 *  copies or substantial portions of the Software. 
 *   
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
 *  SOFTWARE.
 */

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <mpack.h>
#include <unistd.h>

#include "listener.h"
#include "util.h"

// Request typically contains: type, msgid, method, params
// Response contains: type, msgid, error, result
// Notification contains: type, method, params
#define MAX_NODE_COUNT 4

static mpack_error_t get_msgid(struct mpack_node_t root, size_t index,
			       uint32_t *dest)
{
	mpack_error_t err;
	mpack_node_t node = mpack_node_array_at(root, index);
	err = mpack_node_error(node);
	if (err != mpack_ok) {
		return err;
	}
	*dest = mpack_node_u32(node);

	return mpack_ok;
}

static mpack_error_t get_str(struct mem_arena *alloc, mpack_node_t root,
			     size_t index, char **dest)
{
	mpack_error_t err;
	mpack_node_t node = mpack_node_array_at(root, index);
	err = mpack_node_error(node);
	if (err != mpack_ok) {
		return err;
	}
	size_t len = mpack_node_strlen(node) + 1;
	char *mem = mem_arena_alloc(alloc, len);
	mpack_node_copy_cstr(node, mem, len);
	err = mpack_node_error(node);
	if (err != mpack_ok) {
		return err;
	}
	*dest = mem;

	return mpack_ok;
}

static mpack_error_t extract_response(struct msg_context *ctx,
				      mpack_node_t root)
{
	mpack_error_t err;

	err = get_msgid(root, 1, &ctx->data.msgid);
	if (err != mpack_ok) {
		return err;
	}

	mpack_node_t node_error = mpack_node_array_at(root, 2);
	err = mpack_node_error(node_error);
	if (err != mpack_ok) {
		return err;
	}
	ctx->data.error = node_error;

	mpack_node_t node_result = mpack_node_array_at(root, 3);
	err = mpack_node_error(node_result);
	if (err != mpack_ok) {
		return err;
	}
	ctx->data.result = node_result;

	return mpack_ok;
}

static mpack_error_t extract_notification(struct msg_context *ctx,
					  mpack_node_t root)
{
	mpack_error_t err;

	err = get_str(ctx->arena, root, 1, &ctx->data.method);
	if (err != mpack_ok) {
		return err;
	}

	mpack_node_t node_params = mpack_node_array_at(root, 2);
	err = mpack_node_error(node_params);
	if (err != mpack_ok) {
		return err;
	}
	ctx->data.params = node_params;

	return mpack_ok;
}

static mpack_error_t extract_data_into_ctx(struct msg_context *ctx,
					   mpack_tree_t *t)
{
	mpack_node_t root = mpack_tree_root(t);

	mpack_node_t node_type = mpack_node_array_at(root, 0);
	if (mpack_node_is_nil(node_type)) {
		return mpack_node_error(node_type);
	}
	ctx->data.type = (unsigned)mpack_node_int(node_type);

	mpack_error_t err = mpack_ok;
	switch (ctx->data.type) {
	case NVIM_RPC_RESPONSE: {
		err = extract_response(ctx, root);
	} break;
	case NVIM_RPC_NOTIFICATION: {
		err = extract_notification(ctx, root);
	} break;
	default: {
		assert(false);
	} break;
	}

	return err;
}

static void in_read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf)
{
	assert(nread < 0);

	mpack_error_t err;
	struct listener *l = stream->data;
	struct msg_context *ctx = l->current_context;
	l->current_context = NULL;

	mpack_node_data_t *mpack_data =
		mem_arena_alloc(ctx->arena, sizeof(*mpack_data) * MAX_NODE_COUNT);
	mpack_tree_t tree;
	mpack_tree_init_pool(&tree, buf->base, (size_t)nread, mpack_data,
			     MAX_NODE_COUNT);
	mpack_tree_parse(&tree);
	assert(mpack_tree_error(&tree) == mpack_ok);

	err = extract_data_into_ctx(ctx, &tree);
	assert(err == mpack_ok);

	switch (ctx->data.type) {
	case NVIM_RPC_RESPONSE: {
		if (l->on_response) {
			l->on_response(ctx);
		}
	} break;
	case NVIM_RPC_NOTIFICATION: {
		if (l->on_notify) {
			l->on_notify(ctx);
		}
	} break;
	default: {
		assert(false);
	} break;
	}

destroy_tree:
	err = mpack_tree_destroy(&tree);
	assert(err == mpack_ok);
}

static void alloc_cb(uv_handle_t *handle, size_t suggested, uv_buf_t *buf)
{
	struct listener *l = handle->data;

	// We allocate new space for memory arena from a memory pool.
	// Then place new arena object at the beginning, and treat the rest of the given mem block
	// as the arena itself.
	struct mem_arena *a = mem_pool_alloc(l->reqpool);
	mem_arena_init(a, a + sizeof(*a), l->reqpool_block_sz - sizeof(*a));
	assert(a);

	struct msg_context *ctx = mem_arena_alloc(a, sizeof(*ctx));
	assert(ctx);
	ctx->arena = a;
	l->current_context = ctx;

	buf->base = mem_arena_alloc(a, suggested);
	assert(buf->base);
	buf->len = suggested;
}

void listener_init(struct listener *l, uv_loop_t *loop,
		   struct mem_stack *glob_alloc, struct mem_pool *reqpool,
		   size_t reqpool_block_sz, fn_handler on_response,
		   fn_handler on_notify)
{
	memset(l, 0, sizeof(*l));
	l->reqpool = reqpool;
	l->reqpool_block_sz = reqpool_block_sz;
	l->glob_alloc = glob_alloc;
	l->loop = loop;
	l->on_response = on_response;
	l->on_notify = on_notify;

	int err = 0;

	uv_tty_t *tty_in = mem_stack_alloc(l->glob_alloc, sizeof(*tty_in));
	err = uv_tty_init(loop, tty_in, STDIN_FILENO, 0);
	assert(!err);
	tty_in->data = l;
	l->in = (uv_stream_t *)tty_in;

	uv_read_start(l->in, alloc_cb, in_read_cb);
}
