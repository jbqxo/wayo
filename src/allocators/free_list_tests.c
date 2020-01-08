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

#include <errno.h>

#include "unity/src/unity.h"
#include "free_list.h"

static void can_allocate_and_free_single_element(void)
{
	struct free_list allocator;
	void *mem = malloc(4096);
	free_list_init(&allocator, mem, 4096);

	int *precious_number = NULL;
	int_rc rc = free_list_alloc(&allocator, sizeof(*precious_number),
				    (void **)&precious_number);
	TEST_ASSERT_NOT_NULL(precious_number);
	TEST_ASSERT_EQUAL(RC_OK, rc);

	free_list_free(&allocator, precious_number);
}

static void can_allocate_and_free_many_elements(void)
{
	const size_t SET_SIZE = 1000;

	int_rc rc;
	struct free_list fl;
	void *mem = malloc(SET_SIZE * sizeof(int));

	free_list_init(&fl, mem, SET_SIZE * sizeof(int));

	int *results[SET_SIZE];
	size_t actual_size = 0;
	for (size_t i = 0; i < SET_SIZE; i++) {
		actual_size = i;
		rc = free_list_alloc(&fl, sizeof(int), (void **)&results[i]);
		if (rc == -ENOSPC) {
			break;
		}
		TEST_ASSERT_EQUAL(RC_OK, rc);
		TEST_ASSERT_NOT_NULL(results[i]);

		*results[i] = 0xFF;
	}

	for (size_t i = 0; i < actual_size; i++) {
		free_list_free(&fl, results[i]);
	}
}

static void free_space_can_be_coalesced(void)
{
	TEST_FAIL_MESSAGE("not implemented");
}

static void allocation_policy_is_best_fit(void)
{
	TEST_FAIL_MESSAGE("not implemented");
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
	RUN_TEST(free_space_can_be_coalesced);
	RUN_TEST(allocation_policy_is_best_fit);
	return UNITY_END();
}
