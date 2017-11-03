#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>

#include "object.h"
#include "object_index.h"
#include "object_util_io.h"
#include "object_util_debug.h"
#include "object_util_bitmap.h"

#define DEBUG_PRINT_INDEX	

struct POINTER_NODE_INFO
{
	struct POINTER_NODE pointer_node;
	offset_t write_offset;
	uint32 dirty;
};

struct INDEX_INFO
{
	struct POINTER_NODE root_node;
	uint32 max_level;
	uint32 dirty_root_node;
};

static int obj_index_write_pointer_node(struct OBJECT *p_object, offset_t write_offset, void *pointer_node);

int obj_index_create(struct OBJECT *p_object, enum OBJECT_TYPE_NAME obj_type, uint32 max_node_count)
{
	struct INDEX_INFO *p_index_info = NULL;
	uint32 level	= 0;
	uint32 portion	= max_node_count;
	offset_t root_node_offset = 0;

	if(obj_type != OBJ_TYPE_INDEX)
	{
		printf("Error, This object is not INDEX type \r\n");;
		goto ERR_OUT;
	}

	p_index_info = (struct INDEX_INFO *)malloc(sizeof(struct INDEX_INFO));
	if(p_index_info == NULL)
		goto ERR_OUT;
	memset(p_index_info, 0, sizeof(struct INDEX_INFO));
	
	p_object->obj_info.type				= obj_type;
	p_object->obj_info.cur_node_count	= 0;
	p_object->obj_info.obj_index.max_node_count = max_node_count;
	p_object->obj_info.obj_index.p_root_node= 0;
	p_object->obj_info.obj_index.reserved1	= 0;
	p_object->obj_info.obj_index.reserved2	= 0;

	if(obj_bitmap_init(p_object, 1) < 0)
		goto ERR_OUT;

	if(obj_bitmap_set(p_object, 0, sizeof(struct OBJECT_INFO)) < 0)
		goto ERR_OUT;
	
	if(obj_bitmap_set(p_object, sizeof(struct OBJECT_INFO), p_object->p_bitmap->size_b_map_per_cluster) < 0)
		goto ERR_OUT;

	for(level = 0; ; level++)
	{
		portion = portion / 10;
		if(portion < 10)
			break;
	}
	p_index_info->max_level				= level;
	p_index_info->root_node.level		= level;
	p_index_info->root_node.MAGICNUM	= MAGIC_POINTER_POINTER;
	memset(p_index_info->root_node.pointer, 0, sizeof(offset_t) * NODE_POINTER_CNT);	
	p_object->p_reserved = p_index_info;	

	root_node_offset = obj_bitmap_get_free_offset(p_object, sizeof(struct POINTER_NODE));
	if(root_node_offset == 0)
		goto ERR_OUT;
		
	if(obj_index_write_pointer_node(p_object, root_node_offset, &p_index_info->root_node) < 0)
		goto ERR_OUT;

	if(obj_bitmap_set(p_object, root_node_offset, sizeof(struct POINTER_NODE)) < 0)
		goto ERR_OUT;

	p_object->obj_info.obj_index.p_root_node = root_node_offset;

	obj_write_obj_info(p_object, &p_object->obj_info);
	obj_bitmap_write(p_object, 0, p_object->p_bitmap->p_map);

	return 0;

ERR_OUT:
	printf("Error, Create Index \r\n");
	if(p_index_info != NULL)
		free(p_index_info);	

	return -1;
}

int obj_index_open(struct OBJECT *p_object)
{
	struct INDEX_INFO *p_index_info = NULL;
	uint32 level	= 0;
	uint32 portion	= 0;
	offset_t root_node_offset = 0;

	p_index_info = (struct INDEX_INFO *)malloc(sizeof(struct INDEX_INFO));
	if(p_index_info == NULL)
		goto ERR_OUT;

	memset(p_index_info, 0, sizeof(struct INDEX_INFO));	

	if(obj_read_obj_info(p_object, &p_object->obj_info) < 0)
		goto ERR_OUT;

	if(p_object->obj_info.type != OBJ_TYPE_INDEX)
	{
		printf("Error, This object is not INDEX type \r\n");;
		goto ERR_OUT;
	}

	portion = p_object->obj_info.obj_index.max_node_count;
	for(level = 0; ; level++)
	{
		portion = portion / 10;
		if(portion < 10)
			break;
	}
	p_index_info->max_level = level;

	if(obj_io_read(p_object->p_file, p_object->obj_info.obj_index.p_root_node, &p_index_info->root_node, sizeof(struct POINTER_NODE)) < 0)
		goto ERR_OUT;	

	p_object->p_reserved = p_index_info;
	
	if(obj_bitmap_init(p_object, 0) < 0)
		goto ERR_OUT;

	return 0;

ERR_OUT:
	printf("Error, Open Index \r\n");
	if(p_index_info != NULL)
		free(p_index_info);

	return -1;
}

static int obj_index_write_sub(struct OBJECT *p_object, offset_t write_offset, void *data, uint32 length)
{
	uint32 revised_data_length = 0;
	uint32 data_offset = 0;
	uint32 origin_data_length = length;

	do
	{
		if(origin_data_length > fl_get_cluster_size() - (write_offset % fl_get_cluster_size()))
			revised_data_length = fl_get_cluster_size() - (write_offset % fl_get_cluster_size());
		else 
			revised_data_length = origin_data_length;

		if(obj_io_write(p_object->p_file, write_offset, (char*)data + data_offset, revised_data_length) < 0)
		{
			printf("Error, Write the data. offset : %d, length : %d", write_offset, origin_data_length);
			return -1;
		}

		origin_data_length -= revised_data_length;
		if(origin_data_length <= 0)
			break;

		data_offset = revised_data_length;
		write_offset += (revised_data_length + p_object->p_bitmap->size_b_map_per_cluster);		
	}while(1);

	return 0;
}

static int obj_index_read_sub(struct OBJECT *p_object, offset_t read_offset, void *data, uint32 length)
{
	uint32 revised_data_length = 0;
	uint32 data_offset = 0;

	do
	{
		if(length > fl_get_cluster_size() - (read_offset  % fl_get_cluster_size()))
			revised_data_length = fl_get_cluster_size() - (read_offset % fl_get_cluster_size());
		else 
			revised_data_length = length;

		if(obj_io_read(p_object->p_file, read_offset, (char*)data + data_offset, revised_data_length) < 0)
		{
			printf("Error, Read the data. offset : %d, length : %d", read_offset, length);
			return -1;
		}

		length -= revised_data_length;
		if(length <= 0)
			break;

		data_offset = revised_data_length;
		read_offset += (revised_data_length + p_object->p_bitmap->size_b_map_per_cluster);		
	}while(length > 0);

	return 0;
}

static int obj_index_write_pointer_node(struct OBJECT *p_object, offset_t write_offset, void *pointer_node)
{
	if(obj_index_write_sub(p_object, write_offset, pointer_node, sizeof(struct POINTER_NODE)) < 0)
		return -1;
	
	return 0;
}

static int obj_index_read_pointer_node(struct OBJECT *p_object, offset_t read_offset, void *pointer_node)
{
	if(obj_index_read_sub(p_object, read_offset, pointer_node, sizeof(struct POINTER_NODE)) < 0)
		return -1;

	return 0;
}

static int obj_index_write_data(struct OBJECT *p_object, offset_t write_offset, void *data, uint32 length)
{
	if(obj_index_write_sub(p_object, write_offset, data, length) < 0)
		return -1;

	if(obj_bitmap_set(p_object, write_offset, length) < 0)
	{
		printf("Error, Set the bitmap. offset : %d, length : %d", write_offset, length);
		return -1;
	}

	return 0;
}


static int obj_index_read_data(struct OBJECT *p_object, offset_t read_offset, void *data, uint32 length)
{
	if(obj_index_read_sub(p_object, read_offset, data, length) < 0)
		return -1;

	return 0;
}

// return value 
// -1	: error
// 0	: Ã£?? ????
// 1	: Ã£À½ 
static int obj_index_search_index(struct OBJECT *p_object, uint32 index, struct POINTER_NODE *p_pointer_node, offset_t *p_offset)
{
	uint32 i = 0;
	uint32 div_index = 0;
	offset_t offset = 0;
	offset_t prev_offset = 0;
	struct INDEX_INFO *p_index_info = (struct INDEX_INFO*)p_object->p_reserved;
	struct POINTER_NODE pointer_node;

	pointer_node = p_index_info->root_node;	
	prev_offset = p_object->obj_info.obj_index.p_root_node;
	for(i = 0; i <= p_index_info->max_level; i++)
	{
		if(pointer_node.level > 0 && pointer_node.level <= p_index_info->max_level)
			div_index = (index % (uint32)pow(10, pointer_node.level + 1)) /  (uint32)pow(10, pointer_node.level);
		else if(pointer_node.level == 0)
			div_index = index % 10;
		else
		{
			printf("Error, Failed to search index \r\n");
			return -1;
		}

		offset = pointer_node.pointer[div_index];

// print pointer node.
#ifdef  DEBUG_PRINT_INDEX	
		do
		{			
			uint32 loop = 0;
			
			if(pointer_node.level == 0)
				break;

			printf("Level : %d  || ", pointer_node.level);
			for(loop = 0; loop < 10; loop++)
				printf(" %d ", pointer_node.pointer[loop]);
			printf("\r\n");

		}while(0);
#endif

		if(offset != 0)
		{
			if(pointer_node.level == 0)
			{
				*p_offset		= prev_offset;
				*p_pointer_node	= pointer_node;				
				return 1;
			}

			obj_index_read_pointer_node(p_object, offset, &pointer_node);
			prev_offset = offset;
		}
		else 
		{
			*p_offset = prev_offset;
			*p_pointer_node	= pointer_node;		
			return 0;
		}		
	}

	return 0;
}


static int obj_index_make_pointer(struct OBJECT *p_object, uint32 index, struct POINTER_NODE *p_node, offset_t *p_offset)
{
	int i = 0;
	uint32 level = 0;
	uint32 div_index = 0;
	uint32 arr_index_h = 0;
	uint32 arr_index_l = 0;
	
	offset_t offset = 0;
	offset_t error_clear_offset = 0;
	struct POINTER_NODE_INFO arr_pointer_node[2];

	level = p_node->level;		

	if(level == 0)
	{
		printf("Error, Already the node be existed.");
		return -1;
	}
		
	if(p_node->pointer[(index % (uint32)pow(10, level + 1)) / (uint32)pow(10, level)] != 0)
	{
		printf("Error, Already the node be existed. \r\n");
		return -1;
	}

	memset(arr_pointer_node, 0, sizeof(struct POINTER_NODE_INFO) * 2);

	arr_pointer_node[level % 2].pointer_node = *p_node;
	arr_pointer_node[level % 2].write_offset = *p_offset;	

	for(i = level - 1; i >= 0; i--)
	{
		offset = obj_bitmap_get_free_offset(p_object, sizeof(struct POINTER_NODE));
		if(offset == 0)
		{
			printf("Error, Failed to alloc the free offset \r\n");
			return -1;
		}

		error_clear_offset = offset; 

		div_index = (index % (uint32)pow(10, i + 2)) / (uint32)pow(10, i + 1);

		arr_index_l = i % 2;
		arr_index_h = (i + 1) % 2;

		arr_pointer_node[arr_index_h].pointer_node.pointer[div_index] = offset;
		arr_pointer_node[arr_index_l].write_offset			= offset;
		arr_pointer_node[arr_index_l].pointer_node.level	= i;
		if(i == 0)
			arr_pointer_node[arr_index_l].pointer_node.MAGICNUM = MAGIC_POINTER_DATA;
		else 
			arr_pointer_node[arr_index_l].pointer_node.MAGICNUM = MAGIC_POINTER_POINTER;

		// ROOT NODE UPDATE !!!!!!!
		if(arr_pointer_node[arr_index_h].pointer_node.level == ((struct INDEX_INFO *)p_object->p_reserved)->root_node.level)
		{
			((struct INDEX_INFO *)p_object->p_reserved)->root_node.pointer[div_index] = offset;
		}

		if(obj_index_write_pointer_node(p_object, arr_pointer_node[arr_index_h].write_offset, &(arr_pointer_node[arr_index_h].pointer_node)))
		{
			printf("Error, Failed to write the pointer node \r\n");
			goto error_clear_bitmap_out;
		}	

		if(obj_bitmap_set(p_object, arr_pointer_node[arr_index_h].write_offset, sizeof(struct POINTER_NODE)) < 0)
		{
			goto error_clear_bitmap_out;
		}

		//?Ì¸? ?Ò´? 
		if(obj_bitmap_set(p_object, offset, sizeof(struct POINTER_NODE)) < 0)
		{
			goto error_clear_bitmap_out;
		}

#ifdef  DEBUG_PRINT_INDEX	
	do{
		uint32 loop = 0;
		printf("Make Level : %d  || ", arr_pointer_node[arr_index_h].pointer_node.level);
		for(loop = 0; loop < 10; loop++)
			printf(" %d ", arr_pointer_node[arr_index_h].pointer_node.pointer[loop]);
		printf("\r\n");
	}while(0);
#endif

		memset(&arr_pointer_node[arr_index_h], 0, sizeof(struct POINTER_NODE_INFO));
	}

	*p_node		= arr_pointer_node[0].pointer_node;
	*p_offset	= arr_pointer_node[0].write_offset;

	return 0;

error_clear_bitmap_out:
	obj_bitmap_clear(p_object, error_clear_offset, sizeof(struct POINTER_NODE));
	return -1;
}

int obj_index_delete(struct OBJECT *p_object, uint32 index)
{
	int searching_result = 0;
	offset_t pointer_offset = 0;
	offset_t data_offset = 0;
	struct POINTER_NODE pointer_node;
	struct DATA_NODE data_node;

	if(obj_check_valid_object(p_object, OBJ_TYPE_INDEX) < 0)
		return -1;

	if(p_object->obj_info.obj_index.max_node_count <= index)
	{
		printf("Error, index is wrong \r\n");
		return -1;
	}

	searching_result = obj_index_search_index(p_object, index, &pointer_node, &pointer_offset);
	if(searching_result > 0)	// Ã£À½
	{	
		if(pointer_node.MAGICNUM != MAGIC_POINTER_DATA)
		{
			printf("Error, Something wrong \r\n");
			return -1;
		}

		data_offset = pointer_node.pointer[index % 10];
		if(obj_io_read(p_object->p_file, data_offset, &data_node, sizeof(struct DATA_NODE)) < 0)
		{
			printf("Error, Failed to read the data \r\n");
			return -1;
		}
		
		if(obj_bitmap_clear(p_object, data_offset, sizeof(struct DATA_NODE) + data_node.data_length) < 0)
		{
			printf("Errror, Failed to clear bitmap \r\n");
			return -1;
		}

		pointer_node.pointer[index % 10] = 0;
		if(obj_index_write_pointer_node(p_object, pointer_offset, &pointer_node) < 0)
		{
			printf("Error, Failed to write the pointer node \r\n");
			return -1;
		}
		
		p_object->obj_info.cur_node_count--;

		return 0;
	}
	
	printf("Error, Failed to delete index : %d \r\n", index);
	return -1;	
}

int obj_index_set(struct OBJECT *p_object, uint32 index, void *data, uint32 length, uint32 b_overwrite)
{
	int searching_result = 0;
	offset_t pointer_offset = 0;
	offset_t data_offset = 0;
	struct POINTER_NODE pointer_node;
	struct DATA_NODE data_node;
	char *p_wrap_data = NULL;
	uint32 wrap_data_length = sizeof(struct DATA_NODE) + length;

	if(obj_check_valid_object(p_object, OBJ_TYPE_INDEX) < 0)
		return -1;

	if(p_object->obj_info.obj_index.max_node_count <= index)
	{
		printf("Error, index is wrong \r\n");
		return -1;
	}

	searching_result = obj_index_search_index(p_object, index, &pointer_node, &pointer_offset);
	if(searching_result < 0) // error
	{
		return -1;
	}
	else if(searching_result > 0)	// Ã£À½
	{	
		if(b_overwrite == 0)	// ?ßº??Ç´? index?? ???? ???? ??À½
		{
			printf("Error, index overlap \r\n");
			return -1;
		}

		if(pointer_node.MAGICNUM != MAGIC_POINTER_DATA)
		{
			printf("Error, Something wrong \r\n");
			return -1;
		}

		data_offset = pointer_node.pointer[index % 10];
		if(obj_io_read(p_object->p_file, data_offset, &data_node, sizeof(struct DATA_NODE)) < 0)
		{
			printf("Error, Failed to read the data \r\n");
			return -1;
		}
		
		if(obj_bitmap_clear(p_object, data_offset, sizeof(struct DATA_NODE) + data_node.data_length) < 0)
		{
			printf("Errror, Failed to clear bitmap \r\n");
			return -1;
		}

		p_object->obj_info.cur_node_count--;
	}
	else	// ?? Ã£À½.
	{
		if(pointer_node.level != 0)
		{
			if(obj_index_make_pointer(p_object, index, &pointer_node, &pointer_offset) < 0)
			{
				printf("Error, Failed to make the pointer node.\r\n");
				return -1;
			}		

			if(pointer_node.MAGICNUM != MAGIC_POINTER_DATA)
			{
				printf("Error, Something wrong \r\n");
				goto error_out;
			}	
			
		}
	}

	p_wrap_data = (char *)malloc(wrap_data_length);
	if(p_wrap_data == NULL)
	{
		printf("Error, Failed to alloc the memory \r\n");
		return -1;
	}
	memset(p_wrap_data, 0, wrap_data_length);
	
	data_node.MAGICNUM = MAGIC_DATA_HEADER;
	data_node.data_length = length;

	memcpy(p_wrap_data, &data_node, sizeof(struct DATA_NODE));
	memcpy(p_wrap_data + sizeof(struct DATA_NODE), data, length);
	
	data_offset = obj_bitmap_get_free_offset(p_object, wrap_data_length);
	if(data_offset == 0)
	{
		printf("Error, Failed to alloc the free offset \r\n");
		goto error_out;
	}

	if(obj_index_write_data(p_object, data_offset, p_wrap_data, wrap_data_length) < 0)
	{
		printf("Error, Failed to write data \r\n");
		goto error_out;
	}

	pointer_node.pointer[index % 10] = data_offset;

#ifdef DEBUG_PRINT_INDEX	
	do{
		uint32 loop = 0;
		printf("Level : %d  || ", pointer_node.level);
		for(loop = 0; loop < 10; loop++)
			printf(" %d ", pointer_node.pointer[loop]);
		printf("\r\n");
	}while(0);
#endif

	//lazy update. 
	//bitmap is already update.
	if(obj_index_write_pointer_node(p_object, pointer_offset, &pointer_node) < 0)
	{
		printf("Error, Failed to write the pointer node \r\n");
		goto error_out;
	}

	p_object->obj_info.cur_node_count++;

	if(p_wrap_data != NULL)
		free(p_wrap_data);

	return 0;

error_out:
	if(p_wrap_data != NULL)
		free(p_wrap_data);

	return -1;
}

int obj_index_get(struct OBJECT *p_object, uint32 index, void *data)
{
	int searching_result = 0;
	offset_t pointer_offset = 0;
	offset_t data_offset = 0;
	struct POINTER_NODE pointer_node;
	struct DATA_NODE data_node;
	char *p_wrap_data = NULL;

	if(obj_check_valid_object(p_object, OBJ_TYPE_INDEX) < 0)
		return -1;

	searching_result = obj_index_search_index(p_object, index, &pointer_node, &pointer_offset);
	if(searching_result < 0) // error
	{
		return -1;
	}
	else if(searching_result > 0)	// Ã£À½
	{	
		data_offset = pointer_node.pointer[index % 10];
		if(data_offset == 0)
		{
			printf("Error, Failed to read offset\r\n");
			return -1;
		}

		if(obj_io_read(p_object->p_file, data_offset, &data_node, sizeof(struct DATA_NODE)) < 0)
		{
			printf("Error, Failed to read the data \r\n");
			return -1;
		}
		
		if(data_node.MAGICNUM != MAGIC_DATA_HEADER)
		{
			printf("Error, Failed to read the data \r\n");
			return -1;
		}

		if(obj_io_read(p_object->p_file, data_offset + sizeof(struct DATA_NODE), data, data_node.data_length))
		{
			printf("Error, Failed to read the data \r\n");
			return -1;
		}

#ifdef DEBUG_PRINT_INDEX	
	do{
		uint32 loop = 0;
		printf("Level : %d  || ", pointer_node.level);
		for(loop = 0; loop < 10; loop++)
			printf(" %d ", pointer_node.pointer[loop]);
		printf("\r\n");
	}while(0);
#endif

		return 0;
	}
	else	
	{
		printf("Error, Index is not avilable.\r\n");
		return -1;	
	}

	return 0;
}

/* Test Code------------------------------------------------------------------------*/
#define INDEX_TEST_FILE_NAME "/index_test.idx"
#define NUM_OF_DATA		8000	// 300?? ?Æ·??? ??Á¤?Ï·Á¸? ?Æ·??? ???Ì½? ???? ?Ê¿?
struct test_data
{
	int data[64];			
};

void test_routine_obj_index(void)
{
	int ret = 0;
	int i = 0, k = 0;
	struct OBJECT *p_obj;
	struct test_data *data = NULL;
	uint32 rev_data[4096] = {0,};
	
	data = (struct test_data*)malloc(sizeof(struct test_data));
	if(data == NULL)
		goto out;
	
	//create the object
	p_obj = obj_create(INDEX_TEST_FILE_NAME, OBJ_TYPE_INDEX, 10000);
	if(p_obj == NULL)
		goto out;
	
	//close the object.
	if(obj_close(p_obj))
		goto out;
	
	//open the object.
	p_obj = obj_open(INDEX_TEST_FILE_NAME, OBJ_TYPE_INDEX);
	if(p_obj == NULL)
		goto out;
	
	//---------------------------------------------------------------------------------
	for(i = 0; i < NUM_OF_DATA; i++)
	{
		memset(data, 0, sizeof(struct test_data));
		data->data[0] = i;
		printf("0. Set Index : %d, data : %d, size : %dbyte \r\n", i, data->data[0], sizeof(struct test_data));
		ret = obj_index_set(p_obj, i, data, sizeof(struct test_data), 0);
		if(ret < 0)
			goto out;
	}
	
	printf("\r\n---------------------------------------------------------------------\r\n");
	for(i = 0; i < NUM_OF_DATA; i++)
	{
		memset(rev_data, 0, 4096);
		ret = obj_index_get(p_obj, i, rev_data);
		if(ret < 0)
			goto out;

		printf("1. Get Index : %d, get data %d \r\n", i, rev_data[0]);
		if(rev_data[0] != i)
		{
			printf("Error, dequeue data \r\n");
			goto out;
		}	
	}

	if(obj_close(p_obj))
		goto out;

	p_obj = obj_open(INDEX_TEST_FILE_NAME, OBJ_TYPE_INDEX);
	if(p_obj == NULL)
		goto out;

	for(i = NUM_OF_DATA - 1; i >= 0; i--)
	{
		memset(rev_data, 0, 4096);
		ret = obj_index_get(p_obj, i, rev_data);
		if(ret < 0)
			goto out;

		printf("2. Get Index : %d, get data %d \r\n", i, rev_data[0]);
		if(rev_data[0] != i)
		{
			printf("Error, Get Index data \r\n");
			goto out;
		}	
	}

	for(i = NUM_OF_DATA / 4; i < NUM_OF_DATA / 2; i++)
	{
		memset(rev_data, 0, 4096);
		ret = obj_index_get(p_obj, i, rev_data);
		if(ret < 0)
			goto out;

		printf("3. Get Index : %d, get data %d \r\n", i, rev_data[0]);
		if(rev_data[0] != i)
		{
			printf("Error, Get Index data \r\n");
			goto out;
		}	
	}

	for(i = 100; i < 200; i++)
	{
		memset(data, 0, sizeof(struct test_data));
		data->data[0] = i;
		printf("4. Overwrite Index : %d, data : %d, size : %dbyte \r\n", i, data->data[0], sizeof(struct test_data));
		ret = obj_index_set(p_obj, i, data, sizeof(struct test_data), 1);
		if(ret < 0)
			goto out;
	}

	for(i = 100; i < 200; i++)
	{
		memset(rev_data, 0, 4096);
		ret = obj_index_get(p_obj, i, rev_data);
		if(ret < 0)
			goto out;

		printf("5. Get Index : %d, get data %d \r\n", i, rev_data[0]);
		if(rev_data[0] != i)
		{
			printf("Error, Get Index data \r\n");
			goto out;
		}	
	}

	for(i = 100; i < NUM_OF_DATA; i++)
	{
		printf("6. Delete Index : %d\r\n", i);
		ret = obj_index_delete(p_obj, i);
		if(ret < 0)
			goto out;
	}

	for(i = NUM_OF_DATA - 1; i >= 100; i--)
	{
		memset(data, 0, sizeof(struct test_data));
		data->data[0] = i;
		printf("7. Overwrite Index : %d, data : %d, size : %dbyte \r\n", i, data[0].data[0], sizeof(struct test_data));
		ret = obj_index_set(p_obj, i, data, sizeof(struct test_data), 1);
		if(ret < 0)
			goto out;
	}

	for(i = 100; i < 200; i++)
	{
		memset(rev_data, 0, 4096);
		ret = obj_index_get(p_obj, i, rev_data);
		if(ret < 0)
			goto out;

		printf("8. Get Index : %d, get data %d \r\n", i, rev_data[0]);
		if(rev_data[0] != i)
		{
			printf("Error, Get Index data \r\n");
			goto out;
		}	
	}
	
	//---------------------------------------------------------------------------------

	//close the object.
	if(obj_close(p_obj))
		goto out;
	
	printf("Index Object Test Complete. \r\n");

	free(data);
	return ;

out:
	obj_close(p_obj);
	free(data);

	printf("Index Object Test Error \r\n");
	getchar();
}
