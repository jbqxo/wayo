/* Copyright (c) 2019 Maxim Lyapin 
 *  
 *  Permission is hereby granted, free of charge, to any person obtaining a copy 
 *  of this software and associated documentation files the (Software""), to deal 
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
 *  SOFTWARE.)
 */

#include <assert.h>
#include <stdint.h>

#include "allocators.h"

// In the stack allocator, we could simply store the cursor in the memory block and increase/decrease it during alloc/free.
// But if we have this header, we may catch some obscure bugs when somebody frees memory not in a LIFO fashion.
struct header {
	void *prev_cursor;

#ifndef NDEBUG
	size_t block_size;
#endif
};

void stack_init(struct mem_stack *s, void *mem, size_t mem_size)
{
	assert(s);
	assert(mem);
	assert(mem_size > 0);

	*s = (struct mem_stack){ .mem = mem,
				 .mem_end = (uintptr_t)mem + mem_size,
				 .cursor = mem };
}

void *stack_aligned_alloc(struct mem_stack *s, size_t size, size_t alignment)
{
	assert(s);

	uintptr_t aligned_addr =
		nearest_aligned_addr((uintptr_t)s->cursor, alignment);

	// If there is not enough space for the header, move to the next aligned position.
	// Inefficient, but I do not see other solutions.
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

void *stack_alloc(struct mem_stack *s, size_t size) {
	return stack_aligned_alloc(s, size, DEFAULT_ALIGNMENT);
}

void stack_free(struct mem_stack *s, void *block)
{
	assert(s);
	assert(block);

	struct header *h = (void*)((uintptr_t)block - sizeof(struct header));
#ifndef NDEBUG
	// It will be awkward if somebody will try to deallocate not in a LIFO fashion.
	assert((uintptr_t)block + h->block_size == (uintptr_t)s->cursor);
#endif
	s->cursor = h->prev_cursor;
}
