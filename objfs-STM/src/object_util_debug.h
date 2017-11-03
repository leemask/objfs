#ifndef __OBJECT_UTIL_DEBUG__
#define	__OBJECT_UTIL_DEBUG__

#define OBJ_DEBUG_MESSAGE_SHOW
#ifdef OBJ_DEBUG_MESSAGE_SHOW
#define OBJ_DEBUG_PRINT(fmt, ...)  printf("\n[%s][%d] " fmt, __FUNCTION__,  __LINE__,  __VA_ARGS__)
#else 
#define OBJ_DEBUG_PRINT(fmt, ...)
#endif

void obj_debug_print_data_header(const char *caller, struct DATA_HEADER *p_data_header, const void *data);

#endif