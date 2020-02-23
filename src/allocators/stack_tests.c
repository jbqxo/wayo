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
