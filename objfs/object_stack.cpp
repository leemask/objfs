#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "object.h"
#include "object_stack.h"
#include "object_util_io.h"
#include "object_util_debug.h"
#include "object_linkedlist.h"

int obj_stk_push(struct OBJECT *p_object, const void *data, uint32 length)
{
	if(obj_check_valid_object(p_object, OBJ_TYPE_STACK) < 0)
		return -1;

	return obj_ll_add(p_object, data, length);
}

static int obj_stk_pop_sub(struct OBJECT *p_object, void *data)
{
	return obj_ll_remove(p_object, data, &(p_object->obj_info.obj_linked_list.p_latest_node), obj_ll_get_prev_node_offset);
}

int obj_stk_pop(struct OBJECT *p_object, void *data)
{
	if(obj_check_valid_object(p_object, OBJ_TYPE_STACK) < 0)
		return -1;

	return obj_stk_pop_sub(p_object, data);
}

/* Test Code*/
#define STACK_TEST_FILE_NAME "/stack_test.stk"
struct test_data
{
	uint32 data[2];
};

void test_routine_obj_stack(void)
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
	p_obj = obj_create(STACK_TEST_FILE_NAME, OBJ_TYPE_STACK, 0);
	if(p_obj == NULL)
		goto out;
	
	//close the object.
	if(obj_close(p_obj))
		goto out;
	
	//open the object.
	p_obj = obj_open(STACK_TEST_FILE_NAME, OBJ_TYPE_STACK);
	if(p_obj == NULL)
		goto out;
	
	//---------------------------------------------------------------------------------

	for( k = 0; k < 8; k++)
	{
		for(i = (k * 128); i < (128 * (k+1)); i++)
		{
			printf("push %d, size : %dbyte \n", data[i].data[0], sizeof(struct test_data));
			ret = obj_stk_push(p_obj, &data[i], sizeof(struct test_data));
			if(ret < 0)
				goto out;
		}

		if(k == 7)
			break;

		printf("\n---------------------------------------------------------------------\n");
		for(i = (128 * (k+1)) - 1; i >= (k * 128); i--)
		{
			ret = obj_stk_pop(p_obj, rev_data);
			if(ret < 0)
				goto out;

			printf("pop %d \n", rev_data[0]);
			if(rev_data[0] != i)
			{
				printf("Error, pop data \n");
				goto out;
			}	
		}
	}

	if(obj_close(p_obj))
		goto out;

	p_obj = obj_open(STACK_TEST_FILE_NAME, OBJ_TYPE_STACK);
	if(p_obj == NULL)
		goto out;

	for(i = (128 * 8) - 1; i >= (7 * 128); i--)
	{
		ret = obj_stk_pop(p_obj, rev_data);
		if(ret < 0)
			goto out;

		printf("pop %d \n", rev_data[0]);
		if(rev_data[0] != i)
		{
			printf("Error, pop data \n");
			goto out;
		}	
	}
	
	if(obj_stk_push(p_obj, &data[0], sizeof(struct test_data)))
		goto out;
	
	if(obj_stk_pop(p_obj, rev_data))
		goto out;

	//---------------------------------------------------------------------------------

	//close the object.
	if(obj_close(p_obj))
		goto out;
	
	return ;
out:
	obj_close(p_obj);
	printf("Stack Object Test Error \n");
	getchar();
}


#define TEST_FILE "/stack_test"
void test_basic(void)
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
	p_obj = obj_create(TEST_FILE, OBJ_TYPE_STACK, 0);
	if(p_obj == NULL)
		goto out;
return;	
	//close the object.
	if(obj_close(p_obj))
		goto out;
	
	//open the object.
	p_obj = obj_open(TEST_FILE, OBJ_TYPE_STACK);
	if(p_obj == NULL)
		goto out;
	
	//---------------------------------------------------------------------------------

	for( i = 0; i < 8; i++)
	{
		printf("push %d, size : %dbyte \n", data[i].data[0], sizeof(struct test_data));
		ret = obj_stk_push(p_obj, &data[i], sizeof(struct test_data));
		if(ret < 0)
			goto out;

		printf("\n---------------------------------------------------------------------\n");
	}

	if(obj_close(p_obj))
		goto out;

	p_obj = obj_open(TEST_FILE, OBJ_TYPE_STACK);
	if(p_obj == NULL)
		goto out;

	for( i = 0; i < 8; i++)
	{
		ret = obj_stk_pop(p_obj, rev_data);
		if(ret < 0)
			goto out;

		printf("pop %d \n", rev_data[0]);
		if(rev_data[0] != i)
		{
			printf("Error, pop data \n");
			goto out;
		}	
	}
	
	//---------------------------------------------------------------------------------

	//close the object.
	if(obj_close(p_obj))
		goto out;
	
	return ;
out:
	obj_close(p_obj);
	printf("Stack Object Test Error \n");
	getchar();
}

