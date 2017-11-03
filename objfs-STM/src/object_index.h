#include "object.h"

#ifndef __OBJECT_INDEX__
#define	__OBJECT_INDEX__

#define NODE_POINTER_CNT			10

#define MAGIC_POINTER_POINTER		0xEFEF1128
#define MAGIC_POINTER_DATA			0xEFEF0924


/*실제로 디스크에 써지는 정보, 맘껏 수정하지 말 것*/
struct POINTER_NODE
{
	uint32 MAGICNUM;
	uint32 level;						
	offset_t pointer[NODE_POINTER_CNT];
};

/*실제로 디스크에 써지는 정보, 맘껏 수정하지 말 것*/
struct DATA_NODE
{
	uint32 MAGICNUM;
	uint32 data_length;
};

int obj_index_create(struct OBJECT *p_object, enum OBJECT_TYPE_NAME obj_type, uint32 max_node_count);
int obj_index_open(struct OBJECT *p_object);

int obj_index_set(struct OBJECT *p_object, uint32 index, void *data, uint32 length, uint32 b_overwrite);
int obj_index_get(struct OBJECT *p_object, uint32 index, void *data);
int obj_index_delete(struct OBJECT *p_object, uint32 index);

void test_routine_obj_index(void);
#endif