#include "object.h"
#include "object_util_io.h"

int obj_io_write(FL_FILE *p_file, uint32 offset, const void *data, uint32 data_length)
{
	int ret = 0;
	
	ret = fl_fseek(p_file, offset, SEEK_SET);
	if(ret < 0)
		return -1;

	//ret = fl_fdirect_write(data, data_length, 1, p_file);
	ret = fl_fwrite(data, data_length, 1, p_file);
	if(ret != data_length)
		return -1;

	fl_fflush(p_file);

	return 0;
}

int obj_io_read(FL_FILE *p_file, uint32 offset, void *data, uint32 data_length)
{
	int ret = 0;
	ret = fl_fseek(p_file, offset, SEEK_SET);
	if(ret < 0)
		return -1;

	//ret = fl_fdirect_read(data, data_length, 1, p_file);
	ret = fl_fread(data, data_length, 1, p_file);
	printf("obj_io_read ret %d , %d\r\n", ret, data_length);
	if(ret != data_length)
		return -1;

	return 0;
}

int obj_io_flush_bitmap(struct OBJECT *p_object, offset_t write_offset)
{
	return obj_io_write(p_object->p_file, write_offset, p_object->p_bitmap, sizeof(struct BITMAP));	
}
