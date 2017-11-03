#include "object.h"
#include "mbed.h"

#ifndef __OBJECT_STACK__
#define	__OBJECT_STACK__

int obj_stk_push(struct OBJECT *p_object, const void *data, uint32 length);
int obj_stk_pop(struct OBJECT *p_object, void *data);

void test_routine_obj_stack(void);
void test_basic(void);
#endif