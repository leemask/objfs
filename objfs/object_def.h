#ifndef __OBJECT_DEF__
#define	__OBJECT_DEF__

#define MAGIC_DATA_HEADER			0xEFEFEFEF
typedef unsigned int offset_t;


enum OBJECT_TYPE_NAME
{
	OBJ_TYPE_STACK = 0,			//stk
	OBJ_TYPE_QUEUE,				//que
	OBJ_TYPE_STREAM,			//stm      
	OBJ_TYPE_INDEX,				//idx
	OBJ_TYPE_STREAM_INDEX,		//sti
	OBJECT_TYPE_NAME_MAX
};

enum OBJECT_INDEX_NODE_TYPE
{
	POINTER_TO_POINTER = 0,
	POINTER_TO_DATA
};


#endif