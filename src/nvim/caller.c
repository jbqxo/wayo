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

#include <uv.h>
#include <mpack.h>

#include "caller.h"
#include "api.h"
#include "storage.h"
#include "util.h"

static void free_write_buffer(uv_write_t *req, int status)
{
	if (status < 0) {
		// TODO(Maxim Lyapin): Log failed write
	}
	xfree(req->data);
}

// Global functions
void nvim_command(struct nvim_api *api, const char *restrict cmd)
{
	// TODO(Maxim Lyapin): Replace with memory pool.
	uv_buf_t *request_buffer = xmalloc(sizeof(*request_buffer));

	mpack_writer_t writer;
	mpack_writer_init_growable(&writer, &request_buffer->base,
				   &request_buffer->len);

	mpack_start_array(&writer, 4);
	mpack_write_int(&writer, NVIM_RPC_REQUEST);
	mpack_write_u32(&writer, nvim_next_msgid(api));
	mpack_write_cstr(&writer, "nvim_command");
	mpack_write_cstr(&writer, cmd);
	mpack_finish_array(&writer);

	if (mpack_writer_destroy(&writer) != mpack_ok) {
		fprintf(stderr, "An error occurred encoding the data\n");
		return;
	}

	uv_write_t wreq;
	wreq.data = request_buffer;
	uv_write(&wreq, api->out, request_buffer, 1, free_write_buffer);
	// TODO(Maxim Lyapin): Free request_buffer.base
}
