#ifndef __OBJECT_IO__
#define	__OBJECT_IO__

#include "fat_defs.h"
#include "fat_filelib.h"



int obj_io_write(FL_FILE *p_file, uint32 offset, const void *data, uint32 data_length);
int obj_io_read(FL_FILE *p_file, uint32 offset, void *data, uint32 data_length);

#endif