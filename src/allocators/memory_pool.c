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

	void *block = (void *)(uintptr_t)nearest_aligned_addr((uintptr_t)mem,
							      elem_align);
	struct node *last = block;
	struct node *next = (void *)(uintptr_t)nearest_aligned_addr(
		(uintptr_t)last + elem_sz, elem_align);

	while ((uintptr_t)next + elem_sz <= (uintptr_t)mem + mem_size) {
		last->next = next;
		last = next;
		next = (void *)(uintptr_t)nearest_aligned_addr(
			(uintptr_t)last + elem_sz, elem_align);
	}
	last->next = NULL;

	*p = (struct memory_pool){ .head = block };
}

void memory_pool_destroy(struct memory_pool *p)
{
	assert(p);
}

void *memory_pool_alloc(struct memory_pool *p)
{
	assert(p);

	struct node *candidate = p->head;
	if (!candidate) {
		return NULL;
	}

	p->head = candidate->next;
	return candidate;
}

void memory_pool_free(struct memory_pool *p, void *block)
{
	assert(p);
	assert(block);

	struct node *node = block;
	node->next = p->head;
	p->head = node;
}
