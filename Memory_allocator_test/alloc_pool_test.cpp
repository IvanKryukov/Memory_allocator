#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "alloc_pool_test.h"

#include "../Memory_allocator/pool_mem_lib.cpp"

#define RET_TEST_PASS(target_res, res, num) 	printf("[%02d] - ", num); if (target_res == res) printf("OK\n"); else { print_error_type(ret); return num; }
#define PTR_TEST_PASS(target_ptr, ptr, num) 	printf("[%02d] - ", num); if (target_ptr == ptr) printf("OK\n"); else { printf("trgt: 0x%zX | crnt: 0x%zX\n", (size_t)target_ptr, (size_t)ptr); return num; }

#define TEST_BUS_DATA_WIDTH	(sizeof(size_t))

#define POOL_SIZE	(128)	// user defined
#define BLOCK_SIZE	(16)	// user defined

//#define TRACE_MAP_OUT

static void print_error_type( e_allocres err_type)
{
	if (err_type == allres_invalid_pointer)
		printf("invalid pointer\n");

	else if (err_type == allres_invalid_parameter)
		printf("invalid parameter\n");

	else if (err_type == allres_wrong_partition)
		printf("wrong partition\n");

	else if (err_type == allres_too_small_pool)
		printf("allres too small pool\n");

	else if (err_type == allres_too_small_block)
		printf("allres too small block\n");

	else if (err_type == allres_not_aligned_pointer)
		printf("not aligned pointer\n");

	else if (err_type == allres_already_free)
		printf("already free\n");

	else if (err_type == allres_bad_address)
		printf("bad address\n");

	else if (err_type == allres_out_of_pool)
		printf("out of pool\n");

	else
		printf("unknown\n");
}

#if defined TRACE_MAP_OUT
static void print_map( t_pool * pool)
{
	printf(" - Memory map -\n");
	for (size_t i = 0; i < pool->block_count + 1; i++)
	{
		uint8_t * p_jumper = (uint8_t *)pool->base + (i * pool->block_length);
		printf("[%p] - 0x%zX\n", p_jumper, *(size_t *)p_jumper);
	}

	printf("base: [%p]\nhead: [%p]\n", pool->base, pool->head);
}
#endif

int test__pool_init(void)
{
	// - Test 1 - // test for all nuuls and zeros
	e_allocres ret = pool_init(NULL, NULL, 0, 0);
	RET_TEST_PASS(allres_invalid_pointer, ret, 1);

	// - Test 2 - // test for null structure
	uint8_t test_mem[1024];
	ret = pool_init(NULL, test_mem, 0, 0);
	RET_TEST_PASS(allres_invalid_pointer, ret, 2);

	// - Test 3 - // test for null memaory area
	t_pool test_pool;
	ret = pool_init(&test_pool, NULL, 0, 0);
	RET_TEST_PASS(allres_invalid_pointer, ret, 3);

	// - Test 4 - // test for zeroed parameters
	ret = pool_init(&test_pool, test_mem, 0, 0);
	RET_TEST_PASS(allres_invalid_parameter, ret, 4);

	// - Test 5 - // test for zeroed pool length
	ret = pool_init(&test_pool, test_mem, 0, 1);
	RET_TEST_PASS(allres_invalid_parameter, ret, 5);

	// - Test 6 - // test for zeroed block length
	ret = pool_init(&test_pool, test_mem, 1, 0);
	RET_TEST_PASS(allres_invalid_parameter, ret, 6);

	// - Test 7 - // test for block length bigger than pool length
	ret = pool_init(&test_pool, test_mem, 1, 2);
	RET_TEST_PASS(allres_wrong_partition, ret, 7);

	// - Test 8 - // test for POOL length smaller than machine address space + data space
	ret = pool_init(&test_pool, test_mem, TEST_BUS_DATA_WIDTH, TEST_BUS_DATA_WIDTH);
	RET_TEST_PASS(allres_too_small_pool, ret, 8);

	// - Test 9 - // test for BLOCK length smaller than machine address space + data space
	ret = pool_init(&test_pool, test_mem, TEST_BUS_DATA_WIDTH * 2, TEST_BUS_DATA_WIDTH);
	RET_TEST_PASS(allres_too_small_block, ret, 9);

	// - Test 10 - // test for enough space of all lengths
	ret = pool_init(&test_pool, test_mem, TEST_BUS_DATA_WIDTH + 1, TEST_BUS_DATA_WIDTH + 1);
	RET_TEST_PASS(allres_ok, ret, 10);

	// - Test 11 - // test for ordinary situation
	{
		static uint8_t p_test_mem;
		ret = pool_init(&test_pool, &p_test_mem, 24, 6);
		RET_TEST_PASS(allres_ok, ret, 11);
	}

	return 0;
}

int test__pool_alloc(void)
{
	// - Test 1 - //
	void * test_ret = pool_alloc(NULL);
	PTR_TEST_PASS(NULL, test_ret, 1);

	{
		t_pool test_pool;
		static uint8_t test_mem[64];
		memset(test_mem, 0, sizeof(test_mem));

		const uint32_t pool_len = 64;
		const uint32_t block_len = 16;
		pool_init(&test_pool, test_mem, 64, 16);

		// - Test 2 - // test for ordinary situation
		test_ret = pool_alloc(&test_pool);
		PTR_TEST_PASS(test_pool.base, (uint8_t*)test_ret, 2);

		// - Test 3 - // test for ordinary situation
		test_ret = pool_alloc(&test_pool);
		PTR_TEST_PASS((test_pool.base + block_len), (uint8_t*)test_ret, 3);

		// - Test 4 - // test for ordinary situation
		test_ret = pool_alloc(&test_pool);
		PTR_TEST_PASS((test_pool.base + block_len * 2), (uint8_t*)test_ret, 4);

		// - Test 5 - // test for ordinary situation
		test_ret = pool_alloc(&test_pool);
		PTR_TEST_PASS((test_pool.base + block_len * 3), (uint8_t*)test_ret, 5);

		// - Test 6 - // test for overlow
		test_ret = pool_alloc(&test_pool);
		PTR_TEST_PASS(NULL, (uint8_t*)test_ret, 6);
	}

	return 0;
}

int test__pool_free(void)
{
	// - Test 1 - //
	e_allocres ret = pool_free(NULL, NULL);
	RET_TEST_PASS(allres_invalid_pointer, ret, 1);

	{
		t_pool test_pool;
		static uint8_t test_mem[POOL_SIZE];
		memset(test_mem, 0, sizeof(test_mem));

		pool_init(&test_pool, test_mem, POOL_SIZE, BLOCK_SIZE);
#if defined TRACE_MAP_OUT
		print_map(&test_pool);
#endif

		void * test_ptr0 = pool_alloc(&test_pool);
#if defined TRACE_MAP_OUT
		print_map(&test_pool);
#endif

		// - Test 2 - //
		e_allocres ret = pool_free(&test_pool, test_pool.base - 1);
		RET_TEST_PASS(allres_out_of_pool, ret, 2);

		// - Test 3 - //
		ret = pool_free(&test_pool, test_pool.base + test_pool.block_count * test_pool.block_length + 1);
		RET_TEST_PASS(allres_out_of_pool, ret, 3);

		// - Test 4 - //
		ret = pool_free(&test_pool, (void*)((uint32_t*)test_ptr0 + 1));
		RET_TEST_PASS(allres_not_aligned_pointer, ret, 4);

		// - Test 5 - //
		ret = pool_free(&test_pool, (void *)(test_pool.base + test_pool.block_count * test_pool.block_length));
		RET_TEST_PASS(allres_already_free, ret, 5);

		void* test_ptr1 = pool_alloc(&test_pool);
#if defined TRACE_MAP_OUT
		print_map(&test_pool);
#endif

		void* test_ptr2 = pool_alloc(&test_pool);
#if defined TRACE_MAP_OUT
		print_map(&test_pool);
#endif

		void* test_ptr3 = pool_alloc(&test_pool);
#if defined TRACE_MAP_OUT
		print_map(&test_pool);
#endif

		void* test_ptr4 = pool_alloc(&test_pool);
#if defined TRACE_MAP_OUT
		print_map(&test_pool);
#endif

		void* test_ptr5 = pool_alloc(&test_pool);
#if defined TRACE_MAP_OUT
		print_map(&test_pool);
#endif

		void* test_ptr6 = pool_alloc(&test_pool);
#if defined TRACE_MAP_OUT
		print_map(&test_pool);
#endif

		void* test_ptr7 = pool_alloc(&test_pool);
#if defined TRACE_MAP_OUT
		print_map(&test_pool);
#endif
		
		ret = pool_free(&test_pool, test_ptr1);
#if defined TRACE_MAP_OUT
		print_map(&test_pool);
#endif
		RET_TEST_PASS(allres_ok, ret, 6);

		ret = pool_free(&test_pool, test_ptr0);
#if defined TRACE_MAP_OUT
		print_map(&test_pool);
#endif
		RET_TEST_PASS(allres_ok, ret, 7);

		ret = pool_free(&test_pool, test_ptr3);
#if defined TRACE_MAP_OUT
		print_map(&test_pool);
#endif
		RET_TEST_PASS(allres_ok, ret, 8);

		ret = pool_free(&test_pool, test_ptr4);
#if defined TRACE_MAP_OUT
		print_map(&test_pool);
#endif
		RET_TEST_PASS(allres_ok, ret, 9);

		ret = pool_free(&test_pool, test_ptr2);
#if defined TRACE_MAP_OUT
		print_map(&test_pool);
#endif
		RET_TEST_PASS(allres_ok, ret, 10);

		ret = pool_free(&test_pool, test_ptr6);
#if defined TRACE_MAP_OUT
		print_map(&test_pool);
#endif
		RET_TEST_PASS(allres_ok, ret, 11);

		ret = pool_free(&test_pool, test_ptr5);
#if defined TRACE_MAP_OUT
		print_map(&test_pool);
#endif
		RET_TEST_PASS(allres_ok, ret, 12); 

		ret = pool_free(&test_pool, test_ptr7);
#if defined TRACE_MAP_OUT
		print_map(&test_pool);
#endif
		RET_TEST_PASS(allres_already_free, ret, 13);

	}

	return 0;
}