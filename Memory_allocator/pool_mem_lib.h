#if !(defined POOL_MEM_LIB_H)
#define POOL_MEM_LIB_H

#include <stdlib.h>
#include <stdint.h>

#define BUS_DATA_WIDTH (sizeof(size_t)) 

typedef struct
{
	uint8_t * base;
	uint8_t * head;
	size_t	  block_length;
	size_t    block_count;
} t_pool;

typedef enum
{ 
	allres_ok,
	allres_invalid_pointer,
	allres_invalid_parameter,
	allres_wrong_partition,
	allres_too_small_pool,
	allres_too_small_block,
	allres_not_aligned_pointer,
	allres_already_free,
	allres_bad_address,
	allres_out_of_pool
} e_allocres;


/*
 * Initialization of memrory pool structure and space markup
 * 
 * @param:	* pool - pointer to origin structure
 * @param:	member - pointer to base address of needed memory space
 * @param:	size_of_pool__in_bytes	- pool size in bytes
 * @param:	size_of_block__in_bytes	- pool size in bytes
 * 
 * @return:	enumerated result
 */
e_allocres  pool_init(t_pool * pool, uint8_t * member, size_t size_of_pool__in_bytes, size_t size_of_block__in_bytes);

/*
 * Allocation of memory block
 * 
 * @param:	* pool - pointer to origin structure
 * 
 * @return:	pointer to allocated block
 */
void *		pool_alloc(t_pool * pool);

/*
 * Memory block release
 * 
 * @param:	* pool - pointer to origin structure
 * @param:	ptr_to_free - pointer to be freed
 * 
 * @return:	enumerated result
 */
e_allocres	pool_free(t_pool * pool, void * ptr_to_free);

#endif
