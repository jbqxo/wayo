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

/**
 * @brief Represents a memory pool object.
 */
struct mem_pool {
	/* A linked list of free blocks . */
	void *head;
};

void mem_pool_init(struct mem_pool *, void *mem, size_t mem_size,
			size_t element_size, size_t element_alignment);
void mem_pool_destroy(struct mem_pool *);
void *mem_pool_alloc(struct mem_pool *);
void mem_pool_free(struct mem_pool *, void *);

struct mem_stack {
	void *mem;
	uintptr_t mem_end;

	void *cursor;
};

void mem_stack_init(struct mem_stack *, void *mem, size_t mem_size);
void *mem_stack_aligned_alloc(struct mem_stack *, size_t size, size_t alignment);
void *mem_stack_alloc(struct mem_stack *, size_t size);
void mem_stack_free(struct mem_stack *, void *block);

