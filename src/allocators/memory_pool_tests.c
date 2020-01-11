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

#include <errno.h>
#include <assert.h>
#include "unity/src/unity.h"

#include "allocators.h"

static const size_t SET_SIZE = 1000;
static struct memory_pool POOL;
static void *MEMORY;

void setUp()
{
	MEMORY = calloc(SET_SIZE, sizeof(int64_t));
	assert(MEMORY);
	memory_pool_init(&POOL, MEMORY, sizeof(int64_t) * SET_SIZE, sizeof(int64_t),
			 sizeof(int64_t));
}

void tearDown()
{
	memory_pool_destroy(&POOL);
}

void can_allocate_and_free_single_element(void)
{
	int *result = memory_pool_alloc(&POOL);
	TEST_ASSERT_NOT_NULL(result);

	*result = -1;

	memory_pool_free(&POOL, result);
}

void can_allocate_and_free_many_elements(void)
{
	int64_t *results[SET_SIZE];
	for (size_t i = 0; i < SET_SIZE; i++) {
		results[i] = memory_pool_alloc(&POOL);
		TEST_ASSERT_NOT_NULL(results[i]);

		*results[i] = -1;
	}

	for (size_t i = 0; i < SET_SIZE; i++) {
		memory_pool_free(&POOL, results[i]);
	}
}

void receive_an_error_when_there_are_no_free_blocks(void)
{
	for (size_t i = 0; i < SET_SIZE; i++) {
		int64_t *result = memory_pool_alloc(&POOL);
		TEST_ASSERT_NOT_NULL(result);

		*result = -1;
	}
	void *mem = memory_pool_alloc(&POOL);
	TEST_ASSERT_NULL(mem);
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(can_allocate_and_free_single_element);
	RUN_TEST(can_allocate_and_free_many_elements);
	RUN_TEST(receive_an_error_when_there_are_no_free_blocks);
	return UNITY_END();
}
