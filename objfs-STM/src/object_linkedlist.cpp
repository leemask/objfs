#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

#include "object.h"
#include "object_linkedlist.h"
#include "object_util_debug.h"
#include "object_util_io.h"

static char *make_data_node(const void *data, uint32 length, offset_t offset_gap)
{
	char *p_wrap_data;
	struct DATA_HEADER data_header;

	data_header.MAGICNUM	= MAGIC_DATA_HEADER;
	data_header.data_length = length;
	data_header.reserved	= 0;
	data_header.offset_gap	= offset_gap;

	p_wrap_data = (char *)malloc(sizeof(struct DATA_HEADER) + length);
	if(p_wrap_data == NULL)
		return NULL;

	memset(p_wrap_data, 0, sizeof(struct DATA_HEADER) + length);
	memcpy(p_wrap_data, &data_header, sizeof(struct DATA_HEADER));
	memcpy(p_wrap_data + sizeof(struct DATA_HEADER), data, length);

	return p_wrap_data;
}

void obj_ll_create(struct OBJECT *p_object, enum OBJECT_TYPE_NAME obj_type)
{	
	p_object->obj_info.type				= obj_type;
	p_object->obj_info.cur_node_count	= 0;

	p_object->obj_info.obj_linked_list.p_latest_node	= 0;
	p_object->obj_info.obj_linked_list.p_oldest_node	= 0;
	p_object->obj_info.obj_linked_list.p_add_offset		= sizeof(struct OBJECT_INFO);
	p_object->obj_info.obj_linked_list.p_node_pointer	= 0;
	
	obj_write_obj_info(p_object, &p_object->obj_info);			
}

int obj_ll_add(struct OBJECT *p_object, const void *add_data, uint32 length)
{
	int ret = 0;
	char *p_wrap_data			= NULL;	
	uint32 wrap_data_length_b	= length + sizeof(struct DATA_HEADER);
	uint32 write_offset			= p_object->obj_info.obj_linked_list.p_add_offset;	
	FL_FILE *p_file				= p_object->p_file;			
	
	p_wrap_data = make_data_node(	add_data, 
									length, 
									write_offset - p_object->obj_info.obj_linked_list.p_latest_node);
	if(p_wrap_data == NULL)
	{
		printf("Error, Failed to alloc the memroy. \r\n");
		return -1;
	}
		
	ret = obj_io_write(p_file, write_offset, p_wrap_data, wrap_data_length_b);
	if(ret < 0)
		goto out;
	
	if(p_object->obj_info.cur_node_count == 0)
	{
		p_object->obj_info.obj_linked_list.p_oldest_node = write_offset;
		p_object->obj_info.obj_linked_list.p_node_pointer = write_offset;
	}

	p_object->obj_info.cur_node_count++;
	p_object->obj_info.obj_linked_list.p_latest_node = write_offset;	
	p_object->obj_info.obj_linked_list.p_add_offset = write_offset + wrap_data_length_b;

	//obj_debug_print_data_header(__FUNCTION__, &data_header, add_data);
	ret = obj_write_obj_info(p_object, &p_object->obj_info);	

out:
	if(p_wrap_data != NULL)
		free(p_wrap_data);

	return ret;
}

int obj_ll_remove(struct OBJECT *p_object, void *return_data, offset_t *p_offset, offset_t (*cal_node_offset)(struct DATA_HEADER *, offset_t))
{
	int ret = 0;	
	offset_t offset = *p_offset;
	struct DATA_HEADER data_header;
	char *p_read_data = NULL;
	FL_FILE *p_file = NULL;

	if(p_object->obj_info.cur_node_count <= 0)
	{
		printf("Error, Empty in list\r\n");
		return 0;
	}

	p_file = p_object->p_file;			
	ret = obj_ll_read_data_header(p_object, offset, &data_header);
	if(ret < 0) {
		printf("err : obj_ll_read_data_header %d\r\n",ret);
		return -1;
	}

	p_read_data = (char*)malloc(data_header.data_length);
	if(p_read_data == NULL)
	{
		printf("Error, Failed to alloc the memory\r\n");
		return -1;
	}

	ret = obj_io_read(p_file, offset + sizeof(struct DATA_HEADER), p_read_data, data_header.data_length);
	memcpy(return_data, p_read_data, data_header.data_length);
	free(p_read_data);
	if(ret < 0 )
	{
		printf("Error, Failed to dequeue the data \r\n");		
		return -1;
	}

	p_object->obj_info.cur_node_count--;		
	p_object->obj_info.obj_linked_list.p_add_offset = p_object->obj_info.obj_linked_list.p_latest_node;
	*p_offset = cal_node_offset(&data_header, offset);
		
	return obj_write_obj_info(p_object, &p_object->obj_info);		
}

int obj_ll_read_data_header(struct OBJECT *p_object, uint32 offset, void *data_header)
{
	return obj_io_read(p_object->p_file, offset, data_header, sizeof(struct DATA_HEADER));
}

offset_t obj_ll_get_prev_node_offset(struct DATA_HEADER *p_data_header, offset_t base_offset)
{
	return base_offset - p_data_header->offset_gap;
}

offset_t obj_ll_get_next_node_offset(struct DATA_HEADER *p_data_header, offset_t base_offset)
{
	return base_offset + sizeof(struct DATA_HEADER) + p_data_header->data_length;
}

