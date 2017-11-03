#ifndef __OBJECT_UTIL_BITMAP__
#define	__OBJECT_UTIL_BITMAP__

#include "object.h"

#define BIT_PER_BYTE				8
#define BYTE_PER_BIT				16
#define ALIGN_16B(X, Y) ((X) % (Y) > 0) ? ((((X)/ (Y)) + 1) * (Y)) :(X)

int obj_bitmap_init(struct OBJECT *p_object, uint32 b_create_bitmap);
int obj_bitmap_destroy(struct OBJECT *p_object);

int obj_bitmap_write(struct OBJECT *p_object, uint32 bitmap_index, void *bitmap);
int obj_bitmap_read(struct OBJECT *p_object, uint32 bitmap_index, void *bitmap);

offset_t obj_bitmap_get_free_offset(struct OBJECT *p_object, uint32 data_length);
offset_t get_free_offset_extend(struct OBJECT *p_object, uint32 data_length);
int obj_bitmap_set(struct OBJECT *p_object, offset_t offset, uint32 data_length);
int obj_bitmap_clear(struct OBJECT *p_object, offset_t offset, uint32 data_length);
int obj_bitmap_test(void);

#endif