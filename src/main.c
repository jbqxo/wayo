#include <assert.h>
#include <stdio.h>

#include "allocators/allocators.h"
#include "nvim/listener.h"
#include "nvim/caller.h"
#include "util.h"

#include "config.h"

int main(void)
{
	size_t globstack_sz = GLOBAL_STACK_ALLOC_SIZE;
	void *globstack_mem = xmalloc(globstack_sz);
	struct mem_stack global_stack;
	mem_stack_init(&global_stack, globstack_mem, globstack_sz);

	size_t reqpool_sz =
		REQUEST_POOL_BLOCKS_COUNT * REQUEST_POOL_BLOCKS_SIZE;
	void *reqpool_mem = mem_stack_alloc(&global_stack, reqpool_sz);
	assert(reqpool_mem);
	struct mem_pool request_pool;
	mem_pool_init(&request_pool, reqpool_mem, reqpool_sz,
			 REQUEST_POOL_BLOCKS_SIZE, REQUEST_POOL_BLOCKS_SIZE);

	struct listener nvim_api;
	listener_init(&nvim_api, uv_default_loop(), &global_stack, &request_pool, reqpool_sz, NULL, NULL);

	uv_run(uv_default_loop(), UV_RUN_DEFAULT);

	return 0;
}
