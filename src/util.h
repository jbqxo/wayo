#pragma once

#include <stdlib.h>
#include <stdio.h>

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define RC_OK 0
typedef int int_rc;

static void *xmalloc(size_t size)
{
	void *mem = malloc(size);
	if (unlikely(!mem)) {
		exit(EXIT_FAILURE);
	}
	return mem;
}

static void *xcalloc(size_t num, size_t size)
{
	void *mem = calloc(num, size);
	if (unlikely(!mem)) {
		exit(EXIT_FAILURE);
	}
	return mem;
}

static void *xrealloc(void *ptr, size_t new_size)
{
	void *mem = realloc(ptr, new_size);
	if (unlikely(!mem)) {
		exit(EXIT_FAILURE);
	}
	return mem;
}

static void (*xfree)(void *ptr) = free;

static void *xaligned_alloc(size_t alignment, size_t size)
{
	void *mem = aligned_alloc(alignment, size);
	if (unlikely(!mem)) {
		exit(EXIT_FAILURE);
	}
	return mem;
}
