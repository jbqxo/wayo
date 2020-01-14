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

#pragma once

#include <threads.h>

#include <uv.h>

#include "allocators/allocators.h"

typedef uint32_t nvim_rpc_msgid;
struct nvim_api {
	nvim_rpc_msgid msg_counter;
	mtx_t msg_counter_lock;

	uv_stream_t *in;
	uv_stream_t *out;
};

// See: https://github.com/msgpack-rpc/msgpack-rpc/blob/e6a28c4b71638b61ea11469917b030df45ef8081/spec.md
enum nvim_rpc_type {
	NVIM_RPC_REQUEST = 0x0,
	NVIM_RPC_RESPONSE = 0x1,
	NVIM_RPC_NOTIFICATION = 0x2,
};

void nvim_init(struct nvim_api *);
void nvim_connect_stdio(uv_loop_t *, struct nvim_api *);
void nvim_on_read(uv_stream_t *, ssize_t nread, const uv_buf_t *);
nvim_rpc_msgid nvim_next_msgid(struct nvim_api *api);
