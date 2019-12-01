#include "catch2/catch.hpp"

extern "C" {
#include "storage.h"
}


TEST_CASE("Add & get one request from the storage") {
  struct request_storage storage;
  request_storage_init(&storage, 1);

  enum storage_rc rc;
  rc = request_storage_create_req(&storage, 0, NULL, NULL);
  REQUIRE(rc == STORAGE_RC_OK);

  struct request *req;
  rc = request_storage_find(&storage, 0, &req);
  REQUIRE(rc == STORAGE_RC_OK);
  REQUIRE(req != NULL);

  request_storage_destroy(&storage);
}

TEST_CASE("Add & get multiple requests from the storage") {
  const size_t size = 10;

  struct request_storage storage;
  request_storage_init(&storage, size);

  enum storage_rc rc;
  for (size_t req_idx = 0; req_idx < size; req_idx++) {
    rc = request_storage_create_req(&storage, req_idx, (request_callback)req_idx, NULL);
    REQUIRE(rc == STORAGE_RC_OK);
  }

  for (size_t req_idx = 0; req_idx < size; req_idx++) {
    struct request *result;
    rc = request_storage_find(&storage, req_idx, &result);
    REQUIRE(rc == STORAGE_RC_OK);
    REQUIRE((size_t)result->callb == req_idx);
  }

  request_storage_destroy(&storage);
}

TEST_CASE("Delete request from the storage") {
  struct request_storage storage;
  request_storage_init(&storage, 1);

  enum storage_rc rc;
  rc = request_storage_create_req(&storage, 0, NULL, NULL);
  REQUIRE(rc == STORAGE_RC_OK);

  struct request *req;
  rc = request_storage_find(&storage, 0, &req);
  REQUIRE(rc == STORAGE_RC_OK);
  REQUIRE(req != NULL);

  rc = request_storage_free_req(&storage, 0);
  REQUIRE(rc == STORAGE_RC_OK);

  rc = request_storage_find(&storage, 0, &req);
  REQUIRE(rc == STORAGE_RC_NOTFOUND);

  request_storage_destroy(&storage);
}

TEST_CASE("Error when search for a request that doesn't exist") {
  struct request_storage storage;
  request_storage_init(&storage, 1);

  enum storage_rc rc;
  rc = request_storage_create_req(&storage, 0, NULL, NULL);
  REQUIRE(rc == STORAGE_RC_OK);

  struct request *req;
  rc = request_storage_find(&storage, 1, &req);
  REQUIRE(rc == STORAGE_RC_NOTFOUND);

  request_storage_destroy(&storage);
}

TEST_CASE("Error when there is no space") {
  struct request_storage storage;
  request_storage_init(&storage, 1);
  
  enum storage_rc rc;
  rc = request_storage_create_req(&storage, 0, NULL, NULL);
  REQUIRE(rc == STORAGE_RC_OK);

  rc = request_storage_create_req(&storage, 1, NULL, NULL);
  REQUIRE(rc == STORAGE_RC_NOSPACE);

  request_storage_destroy(&storage);
}

TEST_CASE("Error on duplicated msgid") {
  struct request_storage storage;
  request_storage_init(&storage, 1);
  
  enum storage_rc rc;
  rc = request_storage_create_req(&storage, 0, NULL, NULL);
  REQUIRE(rc == STORAGE_RC_OK);

  rc = request_storage_create_req(&storage, 0, NULL, NULL);
  REQUIRE(rc == STORAGE_RC_DUPLICATE);

  request_storage_destroy(&storage);
}

