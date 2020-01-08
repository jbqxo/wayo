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
	int_rc rc;
	struct memory_pool pool;

	rc = memory_pool_init(&pool, 1, sizeof(int), sizeof(int));
	TEST_ASSERT_EQUAL(RC_OK, rc);

	int *result = NULL;
	rc = memory_pool_alloc(&pool, (void **)&result);
	TEST_ASSERT_EQUAL(RC_OK, rc);
	TEST_ASSERT_NOT_NULL(result);

	*result = 0xFF;

	memory_pool_free(&pool, result);

	memory_pool_destroy(&pool);
}

void can_allocate_and_free_many_elements(void)
{
	const size_t SET_SIZE = 1000;

	int_rc rc;
	struct memory_pool pool;

	rc = memory_pool_init(&pool, SET_SIZE, sizeof(int), sizeof(int));
	TEST_ASSERT_EQUAL(RC_OK, rc);

	int *results[SET_SIZE];
	for (size_t i = 0; i < SET_SIZE; i++) {
		rc = memory_pool_alloc(&pool, (void **)&results[i]);
		TEST_ASSERT_EQUAL(RC_OK, rc);
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
	int_rc rc;
	struct memory_pool pool;

	rc = memory_pool_init(&pool, 2, sizeof(int), sizeof(int));
	TEST_ASSERT_EQUAL(RC_OK, rc);

	int *results[3];
	rc = memory_pool_alloc(&pool, (void **)&results[0]);
	TEST_ASSERT_EQUAL(RC_OK, rc);
	TEST_ASSERT_NOT_NULL(results[0]);

	rc = memory_pool_alloc(&pool, (void **)&results[1]);
	TEST_ASSERT_EQUAL(RC_OK, rc);
	TEST_ASSERT_NOT_NULL(results[1]);

	rc = memory_pool_alloc(&pool, (void **)&results[2]);
	TEST_ASSERT_EQUAL(-ENOSPC, rc);

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
