#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

#include "fat_defs.h"
#include "fat_filelib.h"

#include "object.h"
#include "object_index.h"
#include "object_linkedlist.h"
#include "object_util_io.h"
#include "object_util_bitmap.h"

static char *obj_ext[5] = {"stk", "que", "stm", "idx", "sti"};

//extern int _filelib_valid ;
extern int get_filelib_valid();
struct OBJECT *obj_create(char *path, enum OBJECT_TYPE_NAME obj_type, uint32 max_count)
{
	FL_FILE *p_file = NULL;
	struct OBJECT *p_object = NULL;
	struct BITMAP *p_bitmap = NULL;

    printf("obj_create = %d\r\n", get_filelib_valid());
    
	p_file = (FL_FILE*)fl_fopen(path, "r");
	if(p_file != NULL)
	{
		printf("Error, Failed to create the file. (Already exist the file) \r\n");
		fl_fclose(p_file);

		return NULL;
	}

	p_file = (FL_FILE*)fl_fopen(path, "w+");
	if(p_file == NULL)
	{
		printf("Error, Failed to create the file \r\n");
		return NULL;
	}

	p_object = (struct OBJECT*)malloc(sizeof(struct OBJECT));
	if(p_object == NULL)
	{
		printf("Error, Failed to alloc the memory \r\n");
		
		if(p_bitmap != NULL)
			free(p_bitmap);

		fl_fclose(p_file);
		return NULL;
	}

	p_object->p_file	= p_file;
	p_object->p_bitmap	= NULL;
	p_object->p_reserved= NULL;

	switch(obj_type)
	{
	case OBJ_TYPE_STACK:
	case OBJ_TYPE_QUEUE:
	case OBJ_TYPE_STREAM:
		obj_ll_create(p_object, obj_type);	
		break;

	case OBJ_TYPE_INDEX:
		obj_index_create(p_object, obj_type, max_count);
		break;

	case OBJ_TYPE_STREAM_INDEX:
		break;

	default:
		printf("Object Type Error. \r\n");
		obj_close(p_object);
		
		return NULL;
	}

	return p_object;
}

struct OBJECT *obj_open(char *path, enum OBJECT_TYPE_NAME obj_type)
{
	FL_FILE *p_file = NULL;
	struct OBJECT *p_object = NULL;
	struct BITMAP *p_bitmap = NULL;

	p_file = (FL_FILE*)fl_fopen(path, "r+");
	if(p_file == NULL)
	{
		printf("Error, Failed to open the file. \r\n");
	}

	p_object = (struct OBJECT*)malloc(sizeof(struct OBJECT));
	if(p_object == NULL)
	{
		printf("Error, Failed to alloc the memory\r\n");
		
		if(p_bitmap != NULL)
			free(p_bitmap);

		fl_fclose(p_file);
		return NULL;
	}

	p_object->p_file	= p_file;
	p_object->p_bitmap	= NULL;
	p_object->p_reserved= NULL;

	switch(obj_type)
	{
	case OBJ_TYPE_STACK:
	case OBJ_TYPE_QUEUE:
	case OBJ_TYPE_STREAM:
		obj_read_obj_info(p_object, &p_object->obj_info);		
		break;

	case OBJ_TYPE_INDEX:
		obj_index_open(p_object);
		break;

	case OBJ_TYPE_STREAM_INDEX:
		break;

	default:
		goto error_obj_type;
	}

	// ?о??? object type?? ?????ڰ? ??��?? object type?? ???? ??ġ?ϴ??? ?˻?
	if(obj_type != p_object->obj_info.type)
		goto error_obj_type;	

	return p_object;

error_obj_type:
	printf("Object Type Error. \r\n");
	obj_close(p_object);
		
	return NULL;
}

int obj_close(struct OBJECT *p_object)
{
	if(p_object == NULL)
		return -1;

	if(p_object->p_file != NULL)
		fl_fclose(p_object->p_file);

	if(p_object->p_bitmap != NULL)
		obj_bitmap_destroy(p_object);
	
	if(p_object->p_reserved != NULL)
		free(p_object->p_reserved);

	free(p_object);
	return 0;	
}

int obj_write_obj_info(struct OBJECT *p_object, void *data)
{
	return obj_io_write(p_object->p_file, 0, data, sizeof(struct OBJECT_INFO));
}

int obj_read_obj_info(struct OBJECT *p_object, void *data)
{
	return obj_io_read(p_object->p_file, 0, data, sizeof(struct OBJECT_INFO));
}

int obj_check_valid_object(struct OBJECT *p_object, enum OBJECT_TYPE_NAME expected_type)
{
	if(p_object == NULL)
	{
		printf("Error, Object is NULL \r\n");
		return -1;
	}

	if(p_object->p_file == NULL)
	{
		printf("Error, File is NULL \r\n");
		return -1;
	}

	if(p_object->obj_info.type != expected_type)
	{	
		printf("Error, This type is not queue object. \r\n");
		return -1;
	}

	return 0;
}
