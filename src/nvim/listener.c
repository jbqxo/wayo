#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <mpack.h>
#include <unistd.h>

#include "nvim/api.h"
#include "listener.h"
#include "util.h"

static void in_read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf)
{
	assert(nread < 0);

	mpack_error_t err;
	struct listener *l = stream->data;

	mpack_node_data_t *mpack_data =
		mem_stack_alloc(l->alloc, sizeof(*mpack_data) * RPC_MAX_NODE_COUNT);
	mpack_tree_t tree;
	mpack_tree_init_pool(&tree, buf->base, (size_t)nread, mpack_data,
			     RPC_MAX_NODE_COUNT);
	mpack_tree_parse(&tree);
	assert(mpack_tree_error(&tree) == mpack_ok);

	mpack_node_t root = mpack_tree_root(&tree);
	mpack_node_t node_type = mpack_node_array_at(root, 0);
	if (mpack_node_is_nil(node_type)) {
		goto failed_to_extract_type;
	}
	enum msg_type type = mpack_node_int(node_type);

	switch (type) {
	case NVIM_RPC_RESPONSE: {
		if (l->on_response) {
			l->on_response(root);
		}
	} break;
	case NVIM_RPC_NOTIFICATION: {
		if (l->on_notify) {
			l->on_notify(root);
		}
	} break;
	default: {
		assert(false);
	} break;
	}

	err = mpack_tree_destroy(&tree);
	assert(err == mpack_ok);

failed_to_extract_type:
	mem_stack_free(l->alloc, mpack_data);
	mem_stack_free(l->alloc, buf->base);
}

static void alloc_cb(uv_handle_t *handle, size_t suggested, uv_buf_t *buf)
{
	struct listener *l = handle->data;
	buf->base = mem_stack_alloc(l->alloc, suggested);
	assert(buf->base);
	buf->len = suggested;
}

void listener_init(struct listener *l, uv_loop_t *loop,
		   struct mem_stack *alloc,
		   fn_handler on_response,
		   fn_handler on_notify)
{
	memset(l, 0, sizeof(*l));
	l->alloc = alloc;
	l->loop = loop;
	l->on_response = on_response;
	l->on_notify = on_notify;

	int err = 0;

	uv_tty_t *tty_in = mem_stack_alloc(alloc, sizeof(*tty_in));
	err = uv_tty_init(loop, tty_in, STDIN_FILENO, 0);
	assert(!err);
	tty_in->data = l;
	l->in = (uv_stream_t *)tty_in;

	uv_read_start(l->in, alloc_cb, in_read_cb);
}
