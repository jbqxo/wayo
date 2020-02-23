#pragma once

#include <threads.h>

#include <mpack.h>
#include <uv.h>

#include "api.h"
#include "allocators/allocators.h"

// Interface to handle specifict type of a message.
typedef void (*fn_handler)(mpack_node_t root);

struct listener {
	uv_loop_t *loop;
	uv_stream_t *in;

	fn_handler on_response;
	fn_handler on_notify;

	struct mem_stack *alloc;
};

void listener_init(struct listener *l, uv_loop_t *loop,
		   struct mem_stack *alloc,
		   fn_handler on_response,
		   fn_handler on_notify);
