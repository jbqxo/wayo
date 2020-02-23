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

void mem_pool_init(struct mem_pool *p, void *mem, size_t mem_size,
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

	*p = (struct mem_pool){ .head = block };
}

void mem_pool_destroy(struct mem_pool *p)
{
	assert(p);
}

void *mem_pool_alloc(struct mem_pool *p)
{
	assert(p);

	struct node *candidate = p->head;
	if (!candidate) {
		return NULL;
	}

	p->head = candidate->next;
	return candidate;
}

void mem_pool_free(struct mem_pool *p, void *block)
{
	assert(p);
	assert(block);

	struct node *node = block;
	node->next = p->head;
	p->head = node;
}
