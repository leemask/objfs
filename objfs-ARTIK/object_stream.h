#include "object.h"

#ifndef __OBJECT_STREAM__
#define	__OBJECT_STREAM__

enum OBJ_STREAM_SEEK{
	OBJ_STREAM_SEEK_CUR = 0,
	OBJ_STREAM_SEEK_SET,
	OBJ_STREAM_SEEK_END 
};

int obj_stm_append(struct OBJECT *p_object, const void *data, uint32 length);
int obj_stm_seek(struct OBJECT *p_object, int count, enum OBJ_STREAM_SEEK base_loc);
int obj_stm_next_traversal(struct OBJECT *p_object, void *data);
int obj_stm_prev_traversal(struct OBJECT *p_object, void *data);


void test_routine_obj_stream(void);
#endif