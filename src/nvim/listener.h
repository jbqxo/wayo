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

#include <mpack.h>
#include <uv.h>

#include "api.h"
#include "allocators/allocators.h"

// Interface to handle specifict type of a message.
typedef void (*fn_handler)(struct msg_context *ctx);

struct listener {
	uv_loop_t *loop;
	uv_stream_t *in;

	fn_handler on_response;
	fn_handler on_notify;

	struct mem_stack *glob_alloc;
	struct mem_pool *reqpool;
	size_t reqpool_block_sz;

	// Context of the request being handled.
	struct msg_context *current_context;
};

void listener_init(struct listener *l, uv_loop_t *loop,
		   struct mem_stack *glob_alloc, struct mem_pool *reqpool,
		   size_t reqpool_block_sz, fn_handler on_response,
		   fn_handler on_notify);
