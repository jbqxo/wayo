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
#include "unity/src/unity.h"

#include "memory_pool.h"

void can_allocate_and_free_single_element(void)
{
	struct memory_pool pool;

	void *mem = calloc(1, sizeof(int));
	memory_pool_init(&pool, mem, sizeof(int), sizeof(int), sizeof(int));

	int *result = memory_pool_alloc(&pool);
	TEST_ASSERT_NOT_NULL(result);

	*result = 0xFF;

	memory_pool_free(&pool, result);

	memory_pool_destroy(&pool);
}

void can_allocate_and_free_many_elements(void)
{
	const size_t SET_SIZE = 1000;

	struct memory_pool pool;

	void *mem = calloc(SET_SIZE, sizeof(int64_t));
	memory_pool_init(&pool, mem, SET_SIZE * sizeof(int64_t), sizeof(int64_t), sizeof(int64_t));

	int64_t *results[SET_SIZE];
	for (size_t i = 0; i < SET_SIZE; i++) {
		results[i] = memory_pool_alloc(&pool);
		TEST_ASSERT_NOT_NULL(results[i]);

		*results[i] = 0xFF;
	}

	for (size_t i = 0; i < SET_SIZE; i++) {
		memory_pool_free(&pool, results[i]);
	}

	memory_pool_destroy(&pool);
}

void receive_an_error_when_there_are_no_free_blocks(void)
{
	struct memory_pool pool;

	void *mem = calloc(2, sizeof(int64_t));
	memory_pool_init(&pool, mem, 2 * sizeof(int64_t), sizeof(int64_t), sizeof(int64_t));

	int64_t *results[3];
	results[0] = memory_pool_alloc(&pool);
	TEST_ASSERT_NOT_NULL(results[0]);

	results[1] = memory_pool_alloc(&pool);
	TEST_ASSERT_NOT_NULL(results[1]);

	results[2] = memory_pool_alloc(&pool);
	TEST_ASSERT_NULL(results[2]);

	memory_pool_destroy(&pool);
}

void setUp()
{
}
void tearDown()
{
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(can_allocate_and_free_single_element);
	RUN_TEST(can_allocate_and_free_many_elements);
	RUN_TEST(receive_an_error_when_there_are_no_free_blocks);
	return UNITY_END();
}
