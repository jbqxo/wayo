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

#include "api.h"
#include "util.h"

// TODO(Maxim Lyapin): Replace general purpose allocator calls

void nvim_init(struct nvim_api *api)
{
	assert(api);

	memset(api, 0, sizeof(*api));
	mtx_init(&api->msg_counter_lock, mtx_plain);
}

void nvim_destroy(struct nvim_api *api)
{
	assert(api);

	mtx_destroy(&api->msg_counter_lock);

	xfree(api->in);
	xfree(api->out);
	if (api->destroy_func) {
		(*api->destroy_func)(api);
	}
}

void nvim_connect_stdio(uv_loop_t *loop, struct nvim_api *api)
{
	// TODO(Maxim Lyapin): Properly handle errors.
	assert(api);

	int error = 0;

	uv_tty_t *tty_in = xmalloc(sizeof(*tty_in));
	error = uv_tty_init(loop, tty_in, 0, 0);
	assert(!error);
	api->in = (uv_stream_t *)tty_in;

	uv_tty_t *tty_out = xmalloc(sizeof(*tty_out));
	error = uv_tty_init(loop, tty_out, 1, 0);
	assert(!error);
	api->out = (uv_stream_t *)tty_out;

	api->destroy_func = NULL;
}

nvim_rpc_msgid nvim_next_msgid(struct nvim_api *api)
{
	mtx_lock(&api->msg_counter_lock);
	nvim_rpc_msgid msgid = api->msg_counter;
	// Overflow is acceptable in this case.
	api->msg_counter++;
	mtx_unlock(&api->msg_counter_lock);

	return msgid;
}

static void handle_response(mpack_node_t *msg_tree_root)
{
	fprintf(stderr,
		"Received new response message, but the handler is not implemented.");
}

static void handle_request(mpack_node_t *msg_tree_root)
{
	fprintf(stderr,
		"Received new request message, but the handler is not implemented.");
}

static void handle_notification(mpack_node_t *msg_tree_root)
{
	fprintf(stderr,
		"Received new notification message, but the handler is not implemented.");
}

void nvim_on_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf)
{
	mpack_error_t err;
	if (nread < 0) {
		// TODO(Maxim Lyapin): Handle errors
		fprintf(stderr, "An error occurred while reading data\n");
		uv_read_stop(stream);
	} else {
		mpack_tree_t tree;
		mpack_tree_init_data(&tree, buf->base, nread);
		mpack_tree_parse(&tree);

		mpack_node_t root = mpack_tree_root(&tree);

		mpack_node_t rpc_type_node = mpack_node_array_at(root, 0);
		if (mpack_node_is_nil(rpc_type_node)) {
			goto destroy_tree;
		}
		enum nvim_rpc_type type = mpack_node_int(rpc_type_node);

		switch (type) {
		case NVIM_RPC_REQUEST: {
			handle_request(&root);
		} break;
		case NVIM_RPC_RESPONSE: {
			handle_response(&root);
		} break;
		case NVIM_RPC_NOTIFICATION: {
			handle_notification(&root);
		} break;
		}

	destroy_tree:
		err = mpack_tree_destroy(&tree);
		assert(err == mpack_ok);
	}

	if (buf->base) {
		xfree(buf->base);
	}
}
