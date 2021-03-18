#include <stdio.h>
#include <stdbool.h>
#include "pool_mem_lib.h"

#define MARK_BUSY	((size_t)(-1))

/*
 *	The structure of a pool is the linked list.
 * 
 *	Pattern:
 *	+===========================+
 *	| address of next free item |	address of current item
 *	| or NULL mark or BUSY mark	|
 *	+ -  -  -  -  -  -  -  -  - +
 *	|			data			|
 *	+---------------------------+
 *	base:	0xXXXXXXXX
 *	head:	0xXXXXXXXX
 * 
 *	Example of a free space:				Example of a fully busy space:			Example of a randomly busy space:
 *	+===========================+			+===========================+			+===========================+
 *	|			0x10			|	0x00	|			FULL			|	0x00	|			FULL			|	0x00
 *	+ -  -  -  -  -  -  -  -  - +			+ -  -  -  -  -  -  -  -  - +			+ -  -  -  -  -  -  -  -  - +
 *	|			data			|			|			data			|			|			data			|
 *	+---------------------------+			+---------------------------+			+---------------------------+
 *	+===========================+			+===========================+			+===========================+
 *	|			0x20			|	0x10	|			FULL			|	0x10	|			0x30			|	0x10
 *	+ -  -  -  -  -  -  -  -  - +			+ -  -  -  -  -  -  -  -  - +			+ -  -  -  -  -  -  -  -  - +
 *	|			data			|			|			data			|			|			data			|
 *	+---------------------------+			+---------------------------+			+---------------------------+
 *	+===========================+			+===========================+			+===========================+
 *	|			0x30			|	0x20	|			FULL			|	0x20	|			FULL			|	0x20
 *	+ -  -  -  -  -  -  -  -  - +			+ -  -  -  -  -  -  -  -  - +			+ -  -  -  -  -  -  -  -  - +
 *	|			data			|			|			data			|			|			data			|
 *	+---------------------------+			+---------------------------+			+---------------------------+
 *	+===========================+			+===========================+			+===========================+
 *	|			0x40			|	0x30	|			FULL			|	0x30	|			0x40			|	0x30
 *	+ -  -  -  -  -  -  -  -  - +			+ -  -  -  -  -  -  -  -  - +			+ -  -  -  -  -  -  -  -  - +
 *	|			data			|			|			data			|			|			data			|
 *	+---------------------------+			+---------------------------+			+---------------------------+
 *	+===========================+			+===========================+			+===========================+
 *	|			null			|	0x40	|			null			|	0x40	|			null			|	0x40
 *	+ -  -  -  -  -  -  -  -  - +			+ -  -  -  -  -  -  -  -  - +			+ -  -  -  -  -  -  -  -  - +
 *	|			data			|			|			data			|			|			data			|
 *	+---------------------------+			+---------------------------+			+---------------------------+
 *	base:	0x00							base:	0x00							base:	0x00
 *	head:	0x00							head:	NULL							head:	0x10
 *
 */

e_allocres pool_init( t_pool * pool, uint8_t * member, size_t size_of_pool__in_bytes, size_t size_of_block__in_bytes)
{
	if ((NULL == pool) || (NULL == member))
		return allres_invalid_pointer;

	if ((0 == size_of_pool__in_bytes) || (0 == size_of_block__in_bytes))
		return allres_invalid_parameter;

	if (size_of_pool__in_bytes < size_of_block__in_bytes)
		return allres_wrong_partition;

	if (size_of_pool__in_bytes < (BUS_DATA_WIDTH + 1)) // Compare the size with (BUS_DATA_WIDTH + 1) because the first (BUS_DATA_WIDTH) bytes are for address and the last 1, 2, ... bytes - for data 
		return allres_too_small_pool;

	if (size_of_block__in_bytes < (BUS_DATA_WIDTH + 1))
		return allres_too_small_block;

	{
		const size_t block_count = (size_of_pool__in_bytes / size_of_block__in_bytes) - 1;
		size_t block_index;

		for (block_index = 0; block_index < block_count; block_index++)
		{
			uint8_t * current_block = member + ((block_index)*size_of_block__in_bytes);
			*((uint8_t **)current_block) = current_block + size_of_block__in_bytes;
			int ret = 0; // TMP!!!
		}

		*((uint8_t **)&member[block_count * size_of_block__in_bytes]) = NULL; // terminating NULL
		pool->base = pool->head = member;
		pool->block_length = size_of_block__in_bytes;
		pool->block_count  = block_count;
	}

	return allres_ok;
}

void * pool_alloc( t_pool * pool) 
{
	void * ret = NULL;

	if ((NULL != pool) && (NULL != pool->head))
	{
		uint8_t * p_current = pool->head;
		pool->head = (*((uint8_t **)(pool->head)));

		ret = p_current;

		if (*(size_t *)p_current != (size_t)NULL)
			*(size_t *)p_current = MARK_BUSY;
	}

	return ret;
}

e_allocres pool_free( t_pool * pool, void * ptr_to_free)
{
	if (NULL == pool)
		return allres_invalid_pointer;

	if (((uint8_t *)ptr_to_free > (pool->base + pool->block_count * pool->block_length) ) || ((uint8_t *)ptr_to_free < pool->base))
		return allres_out_of_pool;

	if ( ( (uint8_t *)ptr_to_free - pool->base) % (pool->block_length) != 0)
		return allres_not_aligned_pointer;
	
	if (MARK_BUSY != *(size_t *)ptr_to_free)
		return allres_already_free;
	
	/*
	 * change the head of the list
	 */
	if (((uint8_t*)ptr_to_free < pool->head) && (NULL != pool->head))
	{
		*((uint8_t **)ptr_to_free) = pool->head;
		pool->head = (uint8_t *)ptr_to_free;
	}

	/*
	 * if every block is busy the current block becomes the head
	 */
	if (NULL == pool->head) 
	{
		*((uint8_t **)ptr_to_free) = pool->base + (pool->block_count) * pool->block_length;
		pool->head = (uint8_t *)ptr_to_free;
	}
	else
	{
		uint8_t * p_jumper = (uint8_t *)ptr_to_free;

		/*
		 * if the block doesn't contain base address it searches for previous blocks with empty spaces behind them "SEEK BACK"
		 */
		if (p_jumper != pool->base) 
		{
			size_t curr_num = (p_jumper - pool->base) / pool->block_length;
			size_t jumper_val = (size_t)p_jumper;
			size_t i;

			for (i = curr_num - 1; i > 0; i--)
			{
				uint8_t * p_val = pool->base + (i * pool->block_length);
				size_t val = *(size_t*)(p_val);

				if (MARK_BUSY != val)
				{
					if (jumper_val != val)
						*(size_t *)p_val = jumper_val;

					break;
				}
			}
		}

		const size_t count_of_right = pool->block_count - ((uint8_t*)ptr_to_free - pool->base) / pool->block_length;
		bool flag_found = false;

		/*
		 * search for a next free item to make a link "SEEK UP"
		 */
		size_t i;
		for (i = 1; i < count_of_right; i++) 
		{
			size_t current_val = (*(size_t*)(p_jumper + pool->block_length * i));

			if (MARK_BUSY != current_val)
			{
				*(size_t*)p_jumper = (size_t)(p_jumper + pool->block_length * i);
				flag_found = true;
				break;
			}
		}

		if (false == flag_found)
		{
			*((uint8_t**)ptr_to_free) = pool->base + (pool->block_count) * pool->block_length;
			flag_found = true;
		}
	}

	int ret = 0; // TMP!!!

	return allres_ok;
}
