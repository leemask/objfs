#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "object.h"
#include "object_stream.h"
#include "object_util_io.h"
#include "object_util_debug.h"
#include "object_linkedlist.h"

int obj_stm_append(struct OBJECT *p_object, const void *data, uint32 length)
{
	if(obj_check_valid_object(p_object, OBJ_TYPE_STREAM) < 0)
		return -1;

	return obj_ll_add(p_object, data, length);
}

int obj_stm_seek(struct OBJECT *p_object, int count, enum OBJ_STREAM_SEEK base_loc)
{
	int i = 0;
	struct DATA_HEADER data_header;
	offset_t offset = 0;
	offset_t break_offset = 0;
	offset_t (*cal_offset)(struct DATA_HEADER *, offset_t) = NULL;

	if(obj_check_valid_object(p_object, OBJ_TYPE_STREAM) < 0)
		return -1;

	switch(base_loc)
	{
	case OBJ_STREAM_SEEK_SET:
		{
			if(count < 0)
				return 0;

			offset = p_object->obj_info.obj_linked_list.p_oldest_node;
			break_offset = p_object->obj_info.obj_linked_list.p_latest_node;
			cal_offset = obj_ll_get_next_node_offset;
		}
		break;
	case OBJ_STREAM_SEEK_END:
		{
			if(count > 0)
				return 0;

			offset = p_object->obj_info.obj_linked_list.p_latest_node;
			break_offset = p_object->obj_info.obj_linked_list.p_oldest_node;
			cal_offset = obj_ll_get_prev_node_offset;
			count *= -1;
		}
		break;
	case OBJ_STREAM_SEEK_CUR:
		{
			offset = p_object->obj_info.obj_linked_list.p_node_pointer;
						
			if(count < 0) 
			{				
				break_offset = p_object->obj_info.obj_linked_list.p_oldest_node;
				cal_offset = obj_ll_get_prev_node_offset;
				count *= -1;
			}					
			else
			{				
				break_offset = p_object->obj_info.obj_linked_list.p_latest_node;
				cal_offset = obj_ll_get_next_node_offset;
			}
		}
		break;
	}

	for(i = 0; i < count; i++)
	{
		if(offset == break_offset)
			break;

		obj_ll_read_data_header(p_object, offset, &data_header);
		offset = cal_offset(&data_header, offset);
	}

	p_object->obj_info.obj_linked_list.p_node_pointer = offset;

	return 0;
}

static int obj_stm_move_traversal(struct OBJECT *p_object, void *data, offset_t (*cal_offset)(struct DATA_HEADER *, offset_t))
{
	struct DATA_HEADER data_header;
	offset_t offset = p_object->obj_info.obj_linked_list.p_node_pointer;

	if(obj_ll_read_data_header(p_object, offset, &data_header))
		return -1;

	if(data_header.MAGICNUM != MAGIC_DATA_HEADER)
	{
		printf("Error, Dataheader is wrong. \n");
		return -1;
	}

	if(obj_io_read(p_object->p_file, offset + sizeof(struct DATA_HEADER), data, data_header.data_length))
		return -1;

	//obj_debug_print_data_header(__FUNCTION__, &data_header, data);

	if(	offset == p_object->obj_info.obj_linked_list.p_latest_node ||
		offset == p_object->obj_info.obj_linked_list.p_oldest_node)
		return 0;

	p_object->obj_info.obj_linked_list.p_node_pointer = cal_offset(&data_header, offset);	
	return 0;
}

//현재 위치의 데이터를 건내주고 다음 포인터로 이동한다. 
int obj_stm_next_traversal(struct OBJECT *p_object, void *data)
{
	if(obj_check_valid_object(p_object, OBJ_TYPE_STREAM) < 0)
		return -1;

	return obj_stm_move_traversal(p_object, data, obj_ll_get_next_node_offset);
}

//현재 위치의 데이터를 건내주고 이전 포인터로 이동한다. 
int obj_stm_prev_traversal(struct OBJECT *p_object, void *data)
{
	if(obj_check_valid_object(p_object, OBJ_TYPE_STREAM) < 0)
		return -1;

	return obj_stm_move_traversal(p_object, data, obj_ll_get_prev_node_offset);
}

/* Test Code*/
#define STREAM_TEST_FILE_NAME "/stream_test.stm"
struct test_data
{
	uint32 data[2];
};

void test_routine_obj_stream(void)
{
	int ret = 0;
	int i = 0, k = 0;
	struct OBJECT *p_obj;
	struct test_data data[1024]	= { 0x0, };
	uint32 rev_data[128] = {0,};

	//Init data
	for( i = 0; i < 1024; i++)
	{
		data[i].data[0] = i;
	}

	//create the object
	p_obj = obj_create(STREAM_TEST_FILE_NAME, OBJ_TYPE_STREAM, 0);
	if(p_obj == NULL)
		goto out;
	
	//close the object.
	if(obj_close(p_obj))
		goto out;
	
	//open the object.
	p_obj = obj_open(STREAM_TEST_FILE_NAME, OBJ_TYPE_STREAM);
	if(p_obj == NULL)
		goto out;
	
	//---------------------------------------------------------------------------------

	for( k = 0; k < 8; k++)
	{
		for(i = (k * 128); i < (128 * (k+1)); i++)
		{
			printf("append %d, size : %dbyte \n", data[i].data[0], sizeof(struct test_data));
			ret = obj_stm_append(p_obj, &data[i], sizeof(struct test_data));
			if(ret < 0)
				goto out;
		}		
	}

	if(obj_close(p_obj))
		goto out;

	p_obj = obj_open(STREAM_TEST_FILE_NAME, OBJ_TYPE_STREAM);
	if(p_obj == NULL)
		goto out;
			
	printf("Base : SEEK_END, Offset : -100 ------------ \n");
	if(obj_stm_seek(p_obj, -100, OBJ_STREAM_SEEK_END))
		goto out;

	for(i = 0 ; i < 10; i++)
	{
		obj_stm_next_traversal(p_obj, rev_data);
		if(i + 923 != rev_data[0])
		{
			printf("Error, Seek and Read Data \n");
			goto out;
		}

		printf("%d, rev_data : %d \n", i, rev_data[0]);
	}
	
	printf("Base : SEEK_CUR, Offset : 0 ------------ \n");
	if(obj_stm_seek(p_obj, 0, OBJ_STREAM_SEEK_CUR))
		goto out;

	for(i = 0 ; i < 10; i++)
	{
		obj_stm_next_traversal(p_obj, rev_data);
		if(i + 933 != rev_data[0])
		{
			printf("Error, Seek and Read Data \n");
			goto out;
		}

		printf("%d, rev_data : %d \n", i, rev_data[0]);
	}	

	printf("Base : SEEK_SET, Offset : 100 ------------ \n");
	if(obj_stm_seek(p_obj, 100, OBJ_STREAM_SEEK_SET))
		goto out;

	for(i = 0 ; i < 10; i++)
	{
		obj_stm_next_traversal(p_obj, rev_data);
		if(i + 100 != rev_data[0])
		{
			printf("Error, Seek and Read Data \n");
			goto out;
		}

		printf("%d, rev_data : %d \n", i, rev_data[0]);
	}	
		
	printf("Base : SEEK_CUR, Offset : 2000 ------------ \n");
	if(obj_stm_seek(p_obj, 2000, OBJ_STREAM_SEEK_SET))
		goto out;

	for(i = 0 ; i < 10; i++)
	{
		obj_stm_next_traversal(p_obj, rev_data);
		if(rev_data[0] != 1023)
		{
			printf("Error, Seek and Read Data \n");
			goto out;
		}

		printf("%d, rev_data : %d \n", i, rev_data[0]);
	}	
	//---------------------------------------------------------------------------------

	//close the object.
	if(obj_close(p_obj))
		goto out;

	return ;
out:
	obj_close(p_obj);
	printf("Stream Object Test Error \n");
	getchar();
}