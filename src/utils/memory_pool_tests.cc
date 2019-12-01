#include "catch2/catch.hpp"

extern "C" {
#include "memory_pool.h"
}

TEST_CASE("Can allocate and free single element") {
  volatile enum memp_rc rc;
  struct memory_pool pool;

  rc = memory_pool_init(&pool, 1, sizeof(int), sizeof(int));
  REQUIRE(rc == MEMP_RC_OK);

  int *result = nullptr;
  rc = memory_pool_alloc(&pool, (void**)&result);
  REQUIRE(rc == MEMP_RC_OK);
  REQUIRE(result != nullptr);

  *result = 0xFF;

  memory_pool_free(&pool, result);

  memory_pool_destroy(&pool);
}

TEST_CASE("Can allocate and free many elements") {
  const size_t SET_SIZE = 1000;

  enum memp_rc rc;
  struct memory_pool pool;

  rc = memory_pool_init(&pool, SET_SIZE, sizeof(int), sizeof(int));
  REQUIRE(rc == MEMP_RC_OK);

  int *results[SET_SIZE];
  for (size_t i = 0; i < SET_SIZE; i++) {
    rc = memory_pool_alloc(&pool, (void**)&results[i]);
    REQUIRE(rc == MEMP_RC_OK);
    REQUIRE(results[i] != nullptr);

    *results[i] = 0xFF;
  }

  for (size_t i = 0; i < SET_SIZE; i++) {
    memory_pool_free(&pool, results[i]);
  }

  memory_pool_destroy(&pool);
}

TEST_CASE("Receive an error when there are no free blocks") {
  enum memp_rc rc;
  struct memory_pool pool;

  rc = memory_pool_init(&pool,2, sizeof(int), sizeof(int));
  REQUIRE(rc == MEMP_RC_OK);

  int *results[3];
  rc = memory_pool_alloc(&pool, (void**)&results[0]);
  REQUIRE(rc == MEMP_RC_OK);
  REQUIRE(results[0] != nullptr);

  rc = memory_pool_alloc(&pool, (void**)&results[1]);
  REQUIRE(rc == MEMP_RC_OK);
  REQUIRE(results[1] != nullptr);

  rc = memory_pool_alloc(&pool, (void**)&results[2]);
  REQUIRE(rc == MEMP_RC_NOBLOCKS);

  memory_pool_destroy(&pool);
}
