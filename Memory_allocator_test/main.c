#include <stdio.h>
#include "alloc_pool_test.h"

int main(void)
{
	printf("*** Unit testing for pool_mem_lib ***\n");

	printf("\n***Testing pool_init ***\n");
	int failed_test_num = test__pool_init();

	if (0 == failed_test_num)
		printf("All tests has been passed!\n");
	else
		printf("Number of failed test: %d\n", failed_test_num);


	printf("\n*** Testing pool_alloc ***\n");
	failed_test_num = test__pool_alloc();

	if (0 == failed_test_num)
		printf("All tests has been passed!\n");
	else
		printf("Number of failed test: %d\n", failed_test_num);


	printf("\n*** Testing pool_free ***\n");
	failed_test_num = test__pool_free();

	if (0 == failed_test_num)
		printf("All tests has been passed!\n");
	else
		printf("Number of failed test: %d\n", failed_test_num);

	return 0;
}