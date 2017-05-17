#include "object.h"
#include "object_util_debug.h"
/*
void obj_debug_print_data_header(const char *caller, struct DATA_HEADER *p_data_header, const void *data)
{
#ifdef OBJ_DEBUG_MESSAGE_SHOW
	do{
		int i = 0;
		
		printf("[caller] : %s, offset of prev node : %04d, data length : %03d |"
			, caller, p_data_header->p_prev_node, p_data_header->data_length);

		for(i = 0; i < p_data_header->data_length / sizeof(uint32); i++)
		{
			printf(" %d ", *((uint32*)data + i));
		}
		printf("\n");
	}while(0);
#endif
	return ;
}
*/