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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "memory_pool.h"

static size_t calc_elem_size(size_t size, size_t alignment)
{
	size_t s = size;

	// An element must be large enough to store an address in it.
	if (s < sizeof(struct memp_free_node)) {
		s = sizeof(struct memp_free_node);
	}

	// TODO(Maxim Lyapin): Check that this is enough to guarantee the alignment of
	// an element.
	if (s % alignment != 0) {
		s += alignment - (s % alignment);
	}

	return s;
}

static void init_linklist(struct memp_linklist *list, void *block,
			  size_t block_size, size_t elem_size)
{
	list->length = block_size / elem_size;
	list->head = block;

	struct memp_free_node *last = block;
	for (int i = 1; i < list->length; i++) {
		struct memp_free_node *node =
			(struct memp_free_node *)(block + i * elem_size);
		last->next = node;
		last = node;
	}
	last->next = NULL;
}

enum memp_rc memory_pool_init(struct memory_pool *p, size_t capacity,
			      size_t element_size, size_t element_alignment)
{
	assert(capacity != 0);
	assert(p);

	p->elem_size = calc_elem_size(element_size, element_alignment);

	size_t req_mem_size = capacity * p->elem_size;
	void *block;
	int rc = posix_memalign((void **)&block, PAGE_SIZE, req_mem_size);
	if (rc != 0) {
		goto failed_to_alloc_mem_block;
	}
	p->mem_block = block;

	p->lists_len = req_mem_size / PAGE_SIZE;
	// In case if allocated space doesn't fall evenly on a memory page boundary:
	p->lists_len += req_mem_size % PAGE_SIZE != 0 ? 1 : 0;

	p->head = malloc(p->lists_len * sizeof(*p->head));
	if (!p->head) {
		goto failed_to_alloc_space_for_heads;
	}

	for (int i = 0; i < p->lists_len; i++) {
		size_t block_size = req_mem_size - i * PAGE_SIZE;
		if (block_size > PAGE_SIZE) {
			block_size = PAGE_SIZE;
		}

		void *block_addr = block + i * PAGE_SIZE;
		init_linklist(&p->head[i], block_addr, block_size,
			      p->elem_size);
	}

	return MEMP_RC_OK;

failed_to_alloc_space_for_heads:
	free(p->mem_block);

failed_to_alloc_mem_block:
	return MEMP_RC_MALLOC_ERR;
}

void memory_pool_destroy(struct memory_pool *p)
{
	assert(p);

	free(p->head);
	free(p->mem_block);
}

enum memp_rc memory_pool_alloc(struct memory_pool *p, void **result)
{
	assert(p);
	assert(result);

	/* The allocator will try to return a new block from the page where there is the least number of free blocks available.
	 * This way there will be less number of memory pages in use. */
	struct memp_linklist list = { NULL, 0 };
	size_t i = 0;
	for (; i < p->lists_len; i++) {
		if (p->head[i].length == 0) {
			continue;
		}

		if (list.length == 0) {
			list = p->head[i];
			continue;
		}

		if (list.length > p->head[i].length) {
			list = p->head[i];
		}
	}
	i--;

	if (list.length == 0) {
		return MEMP_RC_NOBLOCKS;
	}

	void *allocated = list.head;
	p->head[i].head = list.head->next;
	p->head[i].length--;

	memset(allocated, 0, p->elem_size);
	*result = allocated;
	return MEMP_RC_OK;
}

enum memp_rc memory_pool_free(struct memory_pool *p, void *block)
{
	assert(p);
	assert(block);

	int list_num = ((uintptr_t)block - (uintptr_t)p->mem_block) / PAGE_SIZE;
	if (list_num < 0 || list_num >= p->lists_len) {
		return MEMP_RC_INVVAL;
	}

	struct memp_free_node *node = block;
	node->next = p->head[list_num].head;
	p->head[list_num].head = node;
	p->head[list_num].length++;

	return MEMP_RC_OK;
}
