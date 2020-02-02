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

#include <assert.h>
#include <stdlib.h>

#include "unity/src/unity.h"
#include "allocators.h"

#define SET_SIZE 100
#define ELEMENT_SIZE sizeof(max_align_t)

// Take into account the size of the headers.
static const size_t STACK_SIZE =
	SET_SIZE * ELEMENT_SIZE +
// Header size
#ifndef NDEBUG
	SET_SIZE * (sizeof(size_t) + sizeof(void*));
#else
	SET_SIZE * sizeof(void*);
#endif
static void *mem;
static struct mem_stack s;

void setUp(void)
{
	mem = malloc(STACK_SIZE);
	assert(mem);
	mem_stack_init(&s, mem, STACK_SIZE);
}

void tearDown(void)
{
	free(mem);
}

static void can_perform_simple_allocation(void)
{
	int *results[SET_SIZE];
	for (size_t i = 0; i < SET_SIZE; i++) {
		results[i] = mem_stack_alloc(&s, ELEMENT_SIZE);
		TEST_ASSERT_NOT_NULL(results[i])
		*results[i] = -1;
	}

	for (size_t i = SET_SIZE - 1; i > 0; i--) {
		*results[i] = 0;
		mem_stack_free(&s, results[i]);
	}
	*results[0] = 0;
	mem_stack_free(&s, results[0]);
}

static void cant_allocate_more_than_given(void)
{
	int *results[SET_SIZE];
	for (size_t i = 0; i < SET_SIZE; i++) {
		results[i] = mem_stack_alloc(&s, ELEMENT_SIZE);
		TEST_ASSERT_NOT_NULL(results[i])
		*results[i] = -1;
	}

	int *must_fail = mem_stack_alloc(&s, ELEMENT_SIZE);
	TEST_ASSERT_NULL(must_fail)
}

static void allignment_is_respected(void)
{
	int *must_be_aligned = mem_stack_aligned_alloc(&s, ELEMENT_SIZE, 128);
	TEST_ASSERT((uintptr_t)must_be_aligned % 128 == 0)
}

#ifndef NDEBUG
static void assertion_error_when_not_lifo(void)
{
	int *first = mem_stack_alloc(&s, ELEMENT_SIZE);
	TEST_ASSERT_NOT_NULL(first)

	int *second = mem_stack_alloc(&s, ELEMENT_SIZE);
	TEST_ASSERT_NOT_NULL(second)

	int *third = mem_stack_alloc(&s, ELEMENT_SIZE);
	TEST_ASSERT_NOT_NULL(third)

	TEST_MESSAGE("This test MUST fail");
	mem_stack_free(&s, first);
	TEST_FAIL_MESSAGE("We just ruined our stack allocator. Congrats!");
	mem_stack_free(&s, second);
	mem_stack_free(&s, third);
}
#endif


int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(can_perform_simple_allocation);
	RUN_TEST(cant_allocate_more_than_given);
	RUN_TEST(allignment_is_respected);
#ifndef NDEBUG
	RUN_TEST(assertion_error_when_not_lifo);
#endif
	RUN_TEST(can_perform_simple_allocation);
	UNITY_END();
}
