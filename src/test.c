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

// Phew. It's quite a mess isn't it?
// TODO(Maxim Lyapin): Make it the same way as throwtheswitch.org suggests.

#include <stdio.h>

#include "unity/src/unity.h"

#include "utils/memory_pool_tests.c"
#include "nvim/storage_test.c"

void setUp(void)
{
}
void tearDown(void)
{
}

static void util_memory_pool(void)
{
	printf("Testing utils/memory_pool:\n");
	RUN_TEST(can_allocate_and_free_single_element);
	RUN_TEST(can_allocate_and_free_many_elements);
	RUN_TEST(receive_an_error_when_there_are_no_free_blocks);
	puts("\n");
}

static void nvim_storage(void)
{
	printf("Testing nvim/storage:\n");
	RUN_TEST(add_and_get_one_request_from_the_storage);
	RUN_TEST(add_and_get_multiple_request_from_the_storage);
	RUN_TEST(delete_request_from_the_storage);
	RUN_TEST(return_an_err_when_search_for_a_request_that_doesnt_exist);
	RUN_TEST(return_an_err_when_there_is_no_space);
	RUN_TEST(return_an_err_on_duplicated_msgid);
	puts("\n");
}

int main(void)
{
	UNITY_BEGIN();

	util_memory_pool();
	nvim_storage();

	return UNITY_END();
}
