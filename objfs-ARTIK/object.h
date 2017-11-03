#ifndef __OBJECT_HEADER_INFO__
#define	__OBJECT_HEADER_INFO__

#include "fat_defs.h"
#include "fat_filelib.h"

#include "object_def.h"

// OBJECT using Index 
////////////////////////////////////////////////////////////////////////////////////////////
struct OBJECT_INDEX
{
	uint32 max_node_count;
	offset_t p_root_node;
	uint32 reserved1;
	uint32 reserved2;
};

// Object using Linkedlist
////////////////////////////////////////////////////////////////////////////////////////////
struct OBJECT_LINKED_LIST
{
	offset_t p_latest_node;
	offset_t p_oldest_node;	
	offset_t p_add_offset;
	offset_t p_node_pointer;
};

// Bitmap
////////////////////////////////////////////////////////////////////////////////////////////
struct BITMAP
{	
	int  cur_total_bitmap;		
	int  cur_index;				
	
	int  bit_per_byte;			
	int	 size_b_map_per_cluster;	
	int  map_dirty;				
	char *p_map;
};
////////////////////////////////////////////////////////////////////////////////////////////
struct OBJECT_INFO
{
	enum OBJECT_TYPE_NAME type;
	uint32 cur_node_count;
	union {
		struct OBJECT_INDEX	obj_index;
		struct OBJECT_LINKED_LIST obj_linked_list;
	};
};

struct OBJECT
{
	FL_FILE *p_file;		
	struct OBJECT_INFO obj_info;
	struct BITMAP *p_bitmap;
	void *p_reserved;
};

struct OBJECT *obj_create(char *path, enum OBJECT_TYPE_NAME obj_type, uint32 max_count);
struct OBJECT *obj_open(char *path, enum OBJECT_TYPE_NAME obj_type);
int obj_close(struct OBJECT *p_object);

int obj_write_obj_info(struct OBJECT *p_object, void *data);
int obj_read_obj_info(struct OBJECT *p_object, void *data);
int obj_check_valid_object(struct OBJECT *p_object, enum OBJECT_TYPE_NAME expected_type);

#endif