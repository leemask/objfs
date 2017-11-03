#ifndef __OBJECT_LINKEDLIST__
#define	__OBJECT_LINKEDLIST__

#include "fat_filelib.h"

struct DATA_HEADER
{
	uint32 MAGICNUM;
	uint32 data_length;
	offset_t offset_gap;
	offset_t reserved;
};

void obj_ll_create(struct OBJECT *p_object, enum OBJECT_TYPE_NAME obj_type);

int obj_ll_add(struct OBJECT *p_object, const void *add_data, uint32 length);
int obj_ll_remove(struct OBJECT *p_object, void *return_data, offset_t *p_offset, offset_t (*cal_node_offset)(struct DATA_HEADER *, offset_t));

int obj_ll_read_data_header(struct OBJECT *p_object, uint32 offset, void *data_header);

offset_t obj_ll_get_prev_node_offset(struct DATA_HEADER *p_data_header, offset_t base_offset);
offset_t obj_ll_get_next_node_offset(struct DATA_HEADER *p_data_header, offset_t base_offset);
#endif