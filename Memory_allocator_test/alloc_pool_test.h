#if !(defined ALLOC_POOL_TEST_H)
#define ALLOC_POOL_TEST_H

#include "./../Memory_allocator/pool_mem_lib.h"

/*
 * Unit tests for function 'pool_init'
 */
int test__pool_init(void);

/*
 * Unit tests for function 'pool_alloc'
 */
int test__pool_alloc(void);

/*
 * Unit tests for function 'pool_free'
 */
int test__pool_free(void);

#endif