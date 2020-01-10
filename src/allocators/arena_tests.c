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
 *  SOFTWARE.))
 */

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "unity/src/unity.h"

#include "arena.h"

static void can_allocate_all_the_given_memory(void) {
	const size_t set_size = 100;
	const size_t arena_size = sizeof(int64_t) * set_size; 
	void *mem = malloc(arena_size);
	assert(mem);
	mem_arena a;
	arena_init(&a, mem, arena_size);

	for (int i = 0; i < set_size; i++) {
		int64_t *mem = arena_alloc(&a, sizeof(int64_t), 8);
		TEST_ASSERT_NOT_NULL(mem);
		*mem = -1;
	}

	free(mem);
}

static void cant_allocate_more_than_was_given(void) {
	const size_t set_size = 100;
	const size_t arena_size = sizeof(int64_t) * set_size; 
	void *mem = malloc(arena_size);
	assert(mem);
	mem_arena a;
	arena_init(&a, mem, arena_size);

	for (int i = 0; i < set_size; i++) {
		int64_t *mem = arena_alloc(&a, sizeof(int64_t), 8);
		TEST_ASSERT_NOT_NULL(mem);
		*mem = -1;
	}
	mem = arena_alloc(&a, sizeof(int64_t), 64);
	TEST_ASSERT_NULL(mem);

	free(mem);
}

static void allignment_is_respected(void) {
	void *mem = malloc(4096);
	assert(mem);
	mem_arena a;
	arena_init(&a, mem, 4096);

	int16_t *allocated = arena_alloc(&a, sizeof(int16_t), 64);
	TEST_ASSERT_NOT_NULL(allocated);
	*allocated = -1;
	TEST_ASSERT((uintptr_t)allocated % 64 == 0);
}

void setUp() {
}

void tearDown() {
}

int main (void) {
	UNITY_BEGIN();
	RUN_TEST(can_allocate_all_the_given_memory);
	RUN_TEST(cant_allocate_more_than_was_given);
	RUN_TEST(allignment_is_respected);
	UNITY_END();
}
