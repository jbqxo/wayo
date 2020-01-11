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

#include <stddef.h>
#include <threads.h>

#include "util.h"

struct mem_arena {
	void *current_pos;
	// The field represents the last available address.
	void *arena_edge;
};

void arena_init(struct mem_arena *, void *mem, size_t mem_size);
void *arena_alloc(struct mem_arena *, size_t size, size_t alignment);


/* In the current implementation free block accounting is implemented via
 * a linked list of elements pointing to the free blocks; each element is
 * storred in a corresponding free block. */
/**
 * @brief Represents a memory pool object.
 */
struct memory_pool {
	/* A linked list of free blocks . */
	void *head;
	mtx_t lock;
	/* The size of an element. It could be larger than the requested. */
	size_t elem_size;
};

void memory_pool_init(struct memory_pool *, void *mem, size_t mem_size,
			size_t element_size, size_t element_alignment);
void memory_pool_destroy(struct memory_pool *);
void *memory_pool_alloc(struct memory_pool *);
void memory_pool_free(struct memory_pool *, void *);

struct mem_stack {
	void *mem;
	void *mem_end;

	void *cursor;
};

void stack_init(struct mem_stack *, void *mem, size_t mem_size);
void *stack_alloc(struct mem_stack *, size_t size, size_t alignment);
void stack_free(struct mem_stack *, void *block);

