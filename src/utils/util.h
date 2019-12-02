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

#pragma once

#include <stdlib.h>
#include <stdio.h>

#define __noinline __attribute__((noinline))
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define RC_OK 0
typedef int int_rc;

static void *xmalloc(size_t size) {
    void *mem = malloc(size);
    if (unlikely(!mem)) {
	// TODO(Maxim Lyapin): Add proper error handling.
	fprintf(stderr, "Couldn't allocate more memory.");
	exit(EXIT_FAILURE);
    }
    return mem;
}

static void *xcalloc(size_t num, size_t size) {
    void *mem = calloc(num, size);
    if (unlikely(!mem)) {
	// TODO(Maxim Lyapin): Add proper error handling.
	fprintf(stderr, "Couldn't allocate more memory.");
	exit(EXIT_FAILURE);
    }
    return mem;
}

static void *xrealloc(void *ptr, size_t new_size) {
    void *mem = realloc(ptr, new_size);
    if (unlikely(!mem)) {
	// TODO(Maxim Lyapin): Add proper error handling.
	fprintf(stderr, "Couldn't allocate more memory.");
	exit(EXIT_FAILURE);
    }
    return mem;
}

static void (*xfree)(void *ptr) = free;

static void *xaligned_alloc(size_t alignment, size_t size) {
    void *mem = aligned_alloc(alignment, size);
    if (unlikely(!mem)) {
	// TODO(Maxim Lyapin): Add proper error handling.
	fprintf(stderr, "Couldn't allocate more memory.");
	exit(EXIT_FAILURE);
    }
    return mem;
}
