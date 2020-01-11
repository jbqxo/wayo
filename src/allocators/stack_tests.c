/* Copyright c)( 2019 Maxim Lyapin 
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
#include "stack.h"

static void can_perform_simple_allocation(void)
{
	const size_t SET_SIZE = 100;
	// Take in account the size of the headers.
	const size_t STACK_SIZE =
		SET_SIZE * sizeof(int) + SET_SIZE * (sizeof(size_t) + sizeof(void*));

	void *mem = malloc(STACK_SIZE);
	assert(mem);
	struct mem_stack s;
	stack_init(&s, mem, STACK_SIZE);

	int *results[SET_SIZE];
	for (size_t i = 0; i < SET_SIZE; i++) {
		results[i] = stack_alloc(&s, sizeof(int), 4);
		TEST_ASSERT_NOT_NULL(results[i]);
		*results[i] = -1;
	}

	for (size_t i = SET_SIZE - 1; i > 0; i--) {
		*results[i] = 0;
		stack_free(&s, results[i]);
	}
	*results[0] = 0;
	stack_free(&s, results[0]);

	free(mem);
}

static void cant_allocate_more_than_given(void)
{
	const size_t SET_SIZE = 100;
	// Take in account the size of the headers.
	const size_t STACK_SIZE =
		SET_SIZE * sizeof(int) + SET_SIZE * (sizeof(size_t) + sizeof(void*));

	void *mem = malloc(STACK_SIZE);
	assert(mem);
	struct mem_stack s;
	stack_init(&s, mem, STACK_SIZE);

	int *results[SET_SIZE];
	for (size_t i = 0; i < SET_SIZE; i++) {
		results[i] = stack_alloc(&s, sizeof(int), 4);
		TEST_ASSERT_NOT_NULL(results[i]);
		*results[i] = -1;
	}

	int *must_fail = stack_alloc(&s, sizeof(int), 4);
	TEST_ASSERT_NULL(must_fail);
	free(mem);
}

static void allignment_is_respected(void)
{
	void *mem = malloc(256);
	assert(mem);
	struct mem_stack s;
	stack_init(&s, mem, 256);

	int *must_be_aligned = stack_alloc(&s, sizeof(int), 128);
	TEST_ASSERT((uintptr_t)must_be_aligned % 128 == 0);
	free(mem);
}

#ifndef NDEBUG
static void assertion_error_when_not_lifo(void)
{
	void *mem = malloc(256);
	assert(mem);
	struct mem_stack s;
	stack_init(&s, mem, 256);

	int *first = stack_alloc(&s, sizeof(int), 8);
	TEST_ASSERT_NOT_NULL(first);

	int *second = stack_alloc(&s, sizeof(int), 8);
	TEST_ASSERT_NOT_NULL(second);

	int *third = stack_alloc(&s, sizeof(int), 8);
	TEST_ASSERT_NOT_NULL(third);

	TEST_MESSAGE("This test MUST fail");
	stack_free(&s, first);
	TEST_FAIL_MESSAGE("We just ruined our stack allocator. Congrats!");
	stack_free(&s, second);
	stack_free(&s, third);

	free(mem);
}
#endif

void setUp(void)
{
}

void tearDown(void)
{
}

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
