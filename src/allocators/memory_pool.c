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

#include "allocators.h"

/**
 * @brief The structure represents a free node that points to the next free
 * block and is stored in a free block itself.
 */
struct node {
	struct node *next;
};

static size_t calc_elem_size(size_t size, size_t alignment)
{
	// Check that alignment is a value of power of two.
	assert((alignment & -alignment) == alignment);
	size_t s = size;

	// An element must be large enough to store an address in it.
	if (s < sizeof(struct node)) {
		s = sizeof(struct node);
	}

	return (s + alignment - 1) & -alignment;
}

void memory_pool_init(struct memory_pool *p, void *mem, size_t mem_size,
		      size_t elem_sz, size_t elem_align)
{
	assert(mem);
	assert(mem_size >= elem_sz);
	assert(p);

	// An element must be large enough to be a node.
	if (elem_sz < sizeof(struct node)) {
		elem_sz = sizeof(struct node);
	}

	void *block = nearest_aligned_addr(mem, elem_align);
	struct node *last = block;
	struct node *next =
		nearest_aligned_addr((void*)last + elem_sz, elem_align);
	while ((uintptr_t)next + elem_sz <= (uintptr_t)mem + mem_size) {
		last->next = next;
		last = next;
		next = nearest_aligned_addr((void*)last + elem_sz, elem_align);
	}
	last->next = NULL;

	*p = (struct memory_pool){ .head = block, .elem_size = elem_sz };
	mtx_init(&p->lock, mtx_plain);
}

void memory_pool_destroy(struct memory_pool *p)
{
	assert(p);
	mtx_lock(&p->lock);
	mtx_destroy(&p->lock);
}

void *memory_pool_alloc(struct memory_pool *p)
{
	assert(p);

	mtx_lock(&p->lock);
	struct node *candidate = p->head;
	if (!candidate) {
		mtx_unlock(&p->lock);
		return NULL;
	}

	p->head = candidate->next;
	mtx_unlock(&p->lock);
	return candidate;
}

void memory_pool_free(struct memory_pool *p, void *block)
{
	assert(p);
	assert(block);

	struct node *node = block;
	mtx_lock(&p->lock);
	node->next = p->head;
	p->head = node;
	mtx_unlock(&p->lock);
}
