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
#include <unistd.h>
#include <errno.h>

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

int_rc memory_pool_init(struct memory_pool *p, size_t capacity,
			      size_t element_size, size_t element_alignment)
{
	assert(capacity != 0);
	assert(p);

	size_t elem_size = calc_elem_size(element_size, element_alignment);

	size_t req_mem_size = capacity * elem_size;
	void *block = xaligned_alloc(getpagesize(), req_mem_size);
	if (!block) {
		goto failed_to_alloc_mem_block;
	}

	struct memp_free_node *last = block;
	for (int i = 1; i < capacity; i++) {
		struct memp_free_node *node =
			(struct memp_free_node *)(block + i * elem_size);
		last->next = node;
		last = node;
	}
	last->next = NULL;

	*p = (struct memory_pool) {
	    .mem_block = block,
	    .head = block,
	    .elem_size = elem_size
	};
	mtx_init(&p->lock, mtx_plain);

	return RC_OK;

failed_to_alloc_mem_block:
	return -ENOMEM;
}

void memory_pool_destroy(struct memory_pool *p)
{
	assert(p);
	mtx_lock(&p->lock);

	xfree(p->mem_block);
	mtx_destroy(&p->lock);
}

int_rc memory_pool_alloc(struct memory_pool *p, void **result)
{
	assert(p);
	assert(result);

	mtx_lock(&p->lock);
	struct memp_free_node *candidate = p->head;
	if (!candidate) {
	    mtx_unlock(&p->lock);
	    return -ENOSPC;
	}

	p->head = candidate->next;
	mtx_unlock(&p->lock);
	*result = candidate;

	return RC_OK;
}

void memory_pool_free(struct memory_pool *p, void *block)
{
	assert(p);
	assert(block);

	struct memp_free_node *node = block;
	mtx_lock(&p->lock);
	node->next = p->head;
	p->head = node;
	mtx_unlock(&p->lock);
}
