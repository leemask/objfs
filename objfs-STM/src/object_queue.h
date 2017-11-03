#include "object.h"

#ifndef __OBJECT_QUEUE__
#define	__OBJECT_QUEUE__

int obj_que_enqueue(struct OBJECT *p_object, const void *data, uint32 length);
int obj_que_dequeue(struct OBJECT *p_object, void *data);

void test_routine_obj_queue(void);
#endif