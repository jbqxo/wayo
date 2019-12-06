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

#include "unity/src/unity.h"

#include "storage.h"

void add_and_get_one_request_from_the_storage(void)
{
	struct request_storage storage;
	request_storage_init(&storage, 1);

	enum storage_rc rc;
	rc = request_storage_create_req(&storage, 0, NULL, NULL);
	TEST_ASSERT_EQUAL(STORAGE_RC_OK, rc);

	struct request *req;
	rc = request_storage_find(&storage, 0, &req);
	TEST_ASSERT_EQUAL(STORAGE_RC_OK, rc);
	TEST_ASSERT_NOT_NULL(req);

	request_storage_destroy(&storage);
}

void add_and_get_multiple_request_from_the_storage(void)
{
	const size_t size = 10;

	struct request_storage storage;
	request_storage_init(&storage, size);

	enum storage_rc rc;
	for (size_t req_idx = 0; req_idx < size; req_idx++) {
		rc = request_storage_create_req(
			&storage, req_idx, (request_callback)req_idx, NULL);
		TEST_ASSERT_EQUAL(STORAGE_RC_OK, rc);
	}

	for (size_t req_idx = 0; req_idx < size; req_idx++) {
		struct request *result;
		rc = request_storage_find(&storage, req_idx, &result);
		TEST_ASSERT_EQUAL(STORAGE_RC_OK, rc);
		TEST_ASSERT_EQUAL(req_idx, result->callb);
	}

	request_storage_destroy(&storage);
}

void delete_request_from_the_storage(void)
{
	struct request_storage storage;
	request_storage_init(&storage, 1);

	enum storage_rc rc;
	rc = request_storage_create_req(&storage, 0, NULL, NULL);
	TEST_ASSERT_EQUAL(STORAGE_RC_OK, rc);

	struct request *req;
	rc = request_storage_find(&storage, 0, &req);
	TEST_ASSERT_EQUAL(STORAGE_RC_OK, rc);
	TEST_ASSERT_NOT_NULL(req);

	rc = request_storage_free_req(&storage, 0);

	rc = request_storage_find(&storage, 0, &req);
	TEST_ASSERT_EQUAL(STORAGE_RC_NOTFOUND, rc);

	request_storage_destroy(&storage);
}

void return_an_err_when_search_for_a_request_that_doesnt_exist(void)
{
	struct request_storage storage;
	request_storage_init(&storage, 1);

	enum storage_rc rc;
	rc = request_storage_create_req(&storage, 0, NULL, NULL);
	TEST_ASSERT_EQUAL(STORAGE_RC_OK, rc);

	struct request *req;
	rc = request_storage_find(&storage, 1, &req);
	TEST_ASSERT_EQUAL(STORAGE_RC_NOTFOUND, rc);

	request_storage_destroy(&storage);
}

void return_an_err_when_there_is_no_space(void)
{
	struct request_storage storage;
	request_storage_init(&storage, 1);

	enum storage_rc rc;
	rc = request_storage_create_req(&storage, 0, NULL, NULL);
	TEST_ASSERT_EQUAL(STORAGE_RC_OK, rc);

	rc = request_storage_create_req(&storage, 1, NULL, NULL);
	TEST_ASSERT_EQUAL(STORAGE_RC_NOSPACE, rc);

	request_storage_destroy(&storage);
}

void return_an_err_on_duplicated_msgid(void)
{
	struct request_storage storage;
	request_storage_init(&storage, 1);

	enum storage_rc rc;
	rc = request_storage_create_req(&storage, 0, NULL, NULL);
	TEST_ASSERT_EQUAL(STORAGE_RC_OK, rc);

	rc = request_storage_create_req(&storage, 0, NULL, NULL);
	TEST_ASSERT_EQUAL(STORAGE_RC_DUPLICATE, rc);

	request_storage_destroy(&storage);
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
	RUN_TEST(add_and_get_one_request_from_the_storage);
	RUN_TEST(add_and_get_multiple_request_from_the_storage);
	RUN_TEST(delete_request_from_the_storage);
	RUN_TEST(return_an_err_when_search_for_a_request_that_doesnt_exist);
	RUN_TEST(return_an_err_when_there_is_no_space);
	RUN_TEST(return_an_err_on_duplicated_msgid);
	return UNITY_END();
}
