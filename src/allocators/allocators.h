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

#pragma once

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdalign.h>

#include "util.h"

#define DEFAULT_ALIGNMENT alignof(max_align_t)

static inline uintptr_t nearest_aligned_addr(uintptr_t addr, size_t align) {
	// Check that alignment is a value of power of two.
	assert((align & -align) == align);
	return (addr + align - 1) & -align;
}

struct mem_arena {
	void *current_pos;
	// The field represents the last available address.
	uintptr_t edge_addr;
};

void arena_init(struct mem_arena *, void *mem, size_t mem_size);
void *arena_aligned_alloc(struct mem_arena *, size_t size, size_t alignment);
void *arena_alloc(struct mem_arena *, size_t size);


/* In the current implementation free block accounting is implemented via
 * a linked list of elements pointing to the free blocks; each element is
 * storred in a corresponding free block. */
/**
 * @brief Represents a memory pool object.
 */
struct memory_pool {
	/* A linked list of free blocks . */
	void *head;
};

void memory_pool_init(struct memory_pool *, void *mem, size_t mem_size,
			size_t element_size, size_t element_alignment);
void memory_pool_destroy(struct memory_pool *);
void *memory_pool_alloc(struct memory_pool *);
void memory_pool_free(struct memory_pool *, void *);

struct mem_stack {
	void *mem;
	uintptr_t mem_end;

	void *cursor;
};

void stack_init(struct mem_stack *, void *mem, size_t mem_size);
void *stack_aligned_alloc(struct mem_stack *, size_t size, size_t alignment);
void *stack_alloc(struct mem_stack *, size_t size);
void stack_free(struct mem_stack *, void *block);

