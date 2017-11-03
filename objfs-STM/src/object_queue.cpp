#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "object.h"
#include "object_queue.h"
#include "object_util_debug.h"
#include "object_linkedlist.h"

static int obj_que_dequeue_sub(struct OBJECT *p_object, void *data)
{
	int ret = 0;
	int count = 0;
	offset_t offset_gap = 0;
	offset_t *p_oldest_offset = &(p_object->obj_info.obj_linked_list.p_oldest_node);
	
	ret = obj_ll_remove(p_object, data, p_oldest_offset, obj_ll_get_next_node_offset);
	if(ret < 0)
		return ret;

	// check offset of the oldest node, 
	// if ( 4096 + sizeof(OBJECT_INFO) < p_oldest_node )
	//	copy object_info
	//  delete first cluster index of object in fat table.
	//	update object file infomation. 

	count = (*p_oldest_offset - sizeof(struct OBJECT_INFO)) /  fl_get_cluster_size();
	if(count > 0)
	{
		if(fl_invalid_cluster(p_object->p_file, count) < 0)
		{
			printf("Error, Update Cluster information about this file \r\n");
			return -1;
		}
		
		offset_gap = (fl_get_cluster_size() * count);
		p_object->obj_info.obj_linked_list.p_add_offset -= offset_gap;
		p_object->obj_info.obj_linked_list.p_latest_node -= offset_gap;
		p_object->obj_info.obj_linked_list.p_oldest_node -= offset_gap;
		p_object->obj_info.obj_linked_list.p_node_pointer = 0;

		obj_write_obj_info(p_object, &p_object->obj_info);
	}
	
	return ret;
}

int obj_que_enqueue(struct OBJECT *p_object, const void *data, uint32 length)
{	
	if(obj_check_valid_object(p_object, OBJ_TYPE_QUEUE) < 0)
		return -1;

	return obj_ll_add(p_object, data, length);
}

int obj_que_dequeue(struct OBJECT *p_object, void *data)
{
	if(obj_check_valid_object(p_object, OBJ_TYPE_QUEUE) < 0)
		return -1;

	return obj_que_dequeue_sub(p_object, data);
}


/* Test Code*/
#define QUEUE_TEST_FILE_NAME "/queue_test.stk"
struct test_data
{
	uint32 data[2];
};

void test_routine_obj_queue(void)
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
	p_obj = obj_create(QUEUE_TEST_FILE_NAME, OBJ_TYPE_QUEUE, 0);
	if(p_obj == NULL)
		goto out;
	
	//close the object.
	if(obj_close(p_obj))
		goto out;
	
	//open the object.
	p_obj = obj_open(QUEUE_TEST_FILE_NAME, OBJ_TYPE_QUEUE);
	if(p_obj == NULL)
		goto out;
	
	//---------------------------------------------------------------------------------

	for( k = 0; k < 8; k++)
	{
		for(i = (k * 128); i < (128 * (k+1)); i++)
		{
			printf("enqueue %d, size : %dbyte \r\n", data[i].data[0], sizeof(struct test_data));
			ret = obj_que_enqueue(p_obj, &data[i], sizeof(struct test_data));
			if(ret < 0)
				goto out;
		}

		if(k == 7)
			break;

		
		printf("\r\n---------------------------------------------------------------------\r\n");
		for(i = (k * 128); i < (128 * (k+1)); i++)
		{
			ret = obj_que_dequeue(p_obj, rev_data);
			if(ret < 0)
				goto out;

			printf("dequeue %d \r\n", rev_data[0]);
			if(rev_data[0] != i)
			{
				printf("Error, dequeue data \r\n");
				goto out;
			}	
		}
		
	}

	if(obj_close(p_obj))
		goto out;

	p_obj = obj_open(QUEUE_TEST_FILE_NAME, OBJ_TYPE_QUEUE);
	if(p_obj == NULL)
		goto out;

	for(i = (7 * 128); i < (128 * 8); i++)
	{
		ret = obj_que_dequeue(p_obj, rev_data);
		if(ret < 0)
			goto out;

		printf("dequeue %d \r\n", rev_data[0]);
		if(rev_data[0] != i)
		{
			printf("Error, dequeue data \r\n");
			goto out;
		}	
	}
	
	if(obj_que_enqueue(p_obj, &data[0], sizeof(struct test_data)))
		goto out;
	
	if(obj_que_dequeue(p_obj, rev_data))
		goto out;

	//---------------------------------------------------------------------------------

	//close the object.
	if(obj_close(p_obj))
		goto out;
		
	return ;
out:
	obj_close(p_obj);

	printf("Queue Object Test Error \r\n");

	getchar();
}
