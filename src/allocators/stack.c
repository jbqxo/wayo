#include <assert.h>
#include <stdint.h>

#include "allocators.h"

struct header {
	void *prev_cursor;

// We could simply store the cursor in the memory block and increase/decrease it during alloc/free.
// But if we have this header, we may catch some obscure bugs when somebody frees memory not in a LIFO fashion.
#ifndef NDEBUG
	size_t block_size;
#endif
};

void mem_stack_init(struct mem_stack *s, void *mem, size_t mem_size)
{
	assert(s);
	assert(mem);
	assert(mem_size > 0);

	*s = (struct mem_stack){ .mem = mem,
				 .mem_end = (uintptr_t)mem + mem_size,
				 .cursor = mem };
}

void *mem_stack_aligned_alloc(struct mem_stack *s, size_t size, size_t alignment)
{
	assert(s);

	uintptr_t aligned_addr =
		nearest_aligned_addr((uintptr_t)s->cursor, alignment);

	// If there is not enough space for the header, move to the next aligned position.
	size_t free_space = aligned_addr - (uintptr_t)s->cursor;
	while (free_space < sizeof(struct header)) {
		aligned_addr += alignment;
		free_space += alignment;
	}
	struct header *h = (void*)(aligned_addr - sizeof(struct header));

	uintptr_t new_cursor_addr = aligned_addr + size;
	if (new_cursor_addr > s->mem_end) {
		return NULL;
	} else {
#ifndef NDEBUG
		h->block_size = size;
#endif
		h->prev_cursor = s->cursor;
		s->cursor = (void*)new_cursor_addr;
		return (void*)aligned_addr;
	}
}

void *mem_stack_alloc(struct mem_stack *s, size_t size) {
	return mem_stack_aligned_alloc(s, size, DEFAULT_ALIGNMENT);
}

void mem_stack_free(struct mem_stack *s, void *block)
{
	assert(s);
	assert(block);

	struct header *h = (void*)((uintptr_t)block - sizeof(struct header));
#ifndef NDEBUG
	// Check that dealocation is happening in LIFO fashion.
	assert((uintptr_t)block + h->block_size == (uintptr_t)s->cursor);
#endif
	s->cursor = h->prev_cursor;
}
