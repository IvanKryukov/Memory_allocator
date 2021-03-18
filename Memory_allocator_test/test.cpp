#include "test.h"
#include "./../Memory_allocator/pool_mem_lib.h"

int test__pool_init(void)
{
	t_pool test_pool;
	uint8_t test_mem[];
	uint32_t test_block_size;
	uint32_t test_pool_size;

	// - Test 1 - //
	if (0 != pool_init(NULL, test_mem, 0, 0))
		return 1;

	// - Test 2 - //
	if (0 != pool_init(NULL, test_mem, 0, 0))
		return 2;


	return 0;
}

int test__pool_alloc(void)
{

	return 0;
}

int test__pool_free(void)
{

	return 0;
}