#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "object.h"
#include "object_util_bitmap.h"
#include "object_util_io.h"

#define SET_BIT(data, loc)			((data) |=	(0x1 << (loc)))
#define CLEAR_BIT(data, loc)		((data) &=	~(0x1 << (loc)))
#define IS_SET_BIT(data, loc)		((data) &	(0x1 << (loc)))

static void set_bit(char *data, uint32 loc)
{
	SET_BIT(*data, loc);
}

static void clear_bit(char *data, uint32 loc)
{
	CLEAR_BIT(*data, loc);
}

static offset_t get_free_offset(char *p_map, const uint32 size_b_map, uint32 data_length)
{
	uint32 need_bit_num = 0;
	uint32 continued_bit_cnt = 0 ;
	int i = 0, j = 0;
	int save_i, save_j =0;
	const uint32 byte_size_max_offset = BIT_PER_BYTE - 1;

	need_bit_num = data_length / BYTE_PER_BIT;
	if(data_length % BYTE_PER_BIT > 0)
		need_bit_num++;

	// TBD : bit ????À» ???? free area Ã£?? ?Ë°í¸®??À» ?? È¿À²??À¸?? ?????? ?? ??À» ?? ????...
	for(i = 0; i < size_b_map; i++)
	{
		for( j = byte_size_max_offset; j >= 0; j--)
		{
			if(IS_SET_BIT(p_map[i], j) == 0)
			{
				if(continued_bit_cnt == 0)
				{
					save_i = i;
					save_j = j;				
				}
				
				continued_bit_cnt++;
				if(continued_bit_cnt == need_bit_num)
					return ((save_i * BIT_PER_BYTE) + ((save_j - byte_size_max_offset) * (-1))) * BYTE_PER_BIT;
				
			}
			else 
				continued_bit_cnt = 0;
		}
	}
	
	return 0;	
}

#define NOT_INVALID_OFFSET	0xFFFFFFFF



static int set_clear_bitmap(struct BITMAP *p_bitmap, offset_t offset, uint32 data_length, void (*control_bit)(char *,uint32))
{
	uint32 set_bit_num = 0;
	uint32 bitmap_offset_byte = 0;
	uint32 bitmap_offset_bit = 0;
	uint32 offset_16 = 0;
	uint32 size_b_map_per_cluster = p_bitmap->size_b_map_per_cluster;
	int i = 0, j = 0;
	
	set_bit_num = data_length / p_bitmap->bit_per_byte;
	if(data_length % p_bitmap->bit_per_byte > 0)
		set_bit_num++;

	offset_16 = ALIGN_16B(offset, p_bitmap->bit_per_byte);	

	bitmap_offset_byte = (offset_16 / BYTE_PER_BIT) / BIT_PER_BYTE;
	bitmap_offset_bit = (offset_16 / BYTE_PER_BIT) % BIT_PER_BYTE;

	//????È­?? ?Ê¿ä¼º ??À½.
	for(j = bitmap_offset_byte; j < size_b_map_per_cluster; j++)
	{
		for(i = (bitmap_offset_bit - (BIT_PER_BYTE - 1)) * (-1); i >= 0; i--)
		{	
			control_bit(&(p_bitmap->p_map[j]), i);
			p_bitmap->map_dirty = 1;
			set_bit_num--;
			if(set_bit_num == 0)
				break;
		}
		
		if(set_bit_num == 0)
			break;

		bitmap_offset_bit = 0;		
	}

	return 0;	
}

int obj_bitmap_init(struct OBJECT *p_object, uint32 b_create_bitmap)
{
	struct BITMAP *p_bitmap = NULL;

	if(p_object == NULL)
		return -1;

	if(p_object->p_file == NULL)
		return -1;
	
	p_bitmap = (struct BITMAP *)malloc(sizeof(struct BITMAP));
	if(p_bitmap == NULL)
	{
		printf("Error, Create the bitmap \r\n");
		return -1;
	}	
	memset(p_bitmap, 0, sizeof(struct BITMAP));
	

	p_bitmap->bit_per_byte = BYTE_PER_BIT;
	p_bitmap->size_b_map_per_cluster = fl_get_cluster_size() / BYTE_PER_BIT / BIT_PER_BYTE; 	

	p_bitmap->p_map = (char *)malloc(p_bitmap->size_b_map_per_cluster);
	if(p_bitmap->p_map == NULL)
	{
		printf("Error, Create the bitmap \r\n");
		free(p_bitmap);
		return -1;
	}
	memset(p_bitmap->p_map, 0, p_bitmap->size_b_map_per_cluster);
	
	if(b_create_bitmap == 0)
	{
		if(obj_io_read(	p_object->p_file, 
						ALIGN_16B(sizeof(struct OBJECT_INFO), BYTE_PER_BIT), 
						p_bitmap->p_map, p_bitmap->size_b_map_per_cluster) < 0)
		{
			printf("Error, Open the bitmap\r\n");
			return -1;
		}
	}

	p_bitmap->cur_index = 0;	
	p_bitmap->map_dirty = 0;
	p_bitmap->cur_total_bitmap = fl_get_file_length(p_object->p_file) / fl_get_cluster_size();
	if(fl_get_file_length(p_object->p_file) % fl_get_cluster_size() > 0 || fl_get_file_length(p_object->p_file) == 0)
		p_bitmap->cur_total_bitmap++;
	
	p_object->p_bitmap = p_bitmap;
	return 0;
}

int obj_bitmap_destroy(struct OBJECT *p_object)
{
	if(p_object == NULL)
		return -1;

	if(p_object->p_bitmap == NULL)
		return -1;

	if(p_object->p_bitmap->p_map == NULL)
		return -1;

	if(p_object->p_bitmap->map_dirty == 1)
		obj_bitmap_write(p_object, p_object->p_bitmap->cur_index, p_object->p_bitmap->p_map);

	free(p_object->p_bitmap->p_map);

	p_object->p_bitmap->bit_per_byte = 0;
	p_object->p_bitmap->cur_index = 0;
	p_object->p_bitmap->cur_total_bitmap = 0;
	p_object->p_bitmap->map_dirty = 0;
	p_object->p_bitmap->size_b_map_per_cluster = 0;

	free(p_object->p_bitmap);

	return 0;
}

int obj_bitmap_write(struct OBJECT *p_object, uint32 bitmap_index, void *bitmap)
{
	offset_t write_offset = 0;	

	if(bitmap_index == 0)
		write_offset = ALIGN_16B(sizeof(struct OBJECT_INFO), p_object->p_bitmap->bit_per_byte);
	else 
		write_offset = ALIGN_16B(bitmap_index * fl_get_cluster_size(), p_object->p_bitmap->bit_per_byte) ;

	return obj_io_write(p_object->p_file, write_offset, bitmap, p_object->p_bitmap->size_b_map_per_cluster);
}

int obj_bitmap_read(struct OBJECT *p_object, uint32 bitmap_index, void *bitmap)
{
	offset_t read_offset = 0;	

	if(bitmap_index == 0)
		read_offset = ALIGN_16B(sizeof(struct OBJECT_INFO), p_object->p_bitmap->bit_per_byte);
	else 
		read_offset = ALIGN_16B(bitmap_index * fl_get_cluster_size(), p_object->p_bitmap->bit_per_byte) ;

	return obj_io_read(p_object->p_file, read_offset, bitmap, p_object->p_bitmap->size_b_map_per_cluster);
}

int obj_bitmap_sub_set_clear(struct OBJECT *p_object, offset_t offset, uint32 data_length, void (*control_bit)(char *,uint32))
{
	uint32 bitmap_index = offset / fl_get_cluster_size();
	uint32 revised_offset = offset % fl_get_cluster_size();
	int remind_data_length = (int)data_length;
	struct BITMAP *p_bitmap = p_object->p_bitmap;

	if(bitmap_index != p_bitmap->cur_index)
	{
		if(p_bitmap->map_dirty == 1)
		{
			if(obj_bitmap_write(p_object, p_bitmap->cur_index, p_bitmap->p_map))
				return -1;				
		}
		
		if(bitmap_index == p_bitmap->cur_total_bitmap)
		{
			p_bitmap->cur_index = bitmap_index;
			p_bitmap->cur_total_bitmap++;
			
			memset(p_bitmap->p_map, 0, p_bitmap->size_b_map_per_cluster);			
			if(set_clear_bitmap(p_object->p_bitmap, 
								0, 
								ALIGN_16B(p_bitmap->size_b_map_per_cluster, p_bitmap->bit_per_byte), 
								set_bit))
				return -1;
		}
		else 
		{
			obj_bitmap_read(p_object, bitmap_index, p_bitmap->p_map);				
			p_bitmap->cur_index = bitmap_index;
		}

		p_bitmap->map_dirty = 0;
	}
	
	while(remind_data_length >= 0)
	{
		if(remind_data_length > fl_get_cluster_size() - revised_offset)
			data_length = fl_get_cluster_size() - revised_offset;
		else
			data_length = remind_data_length;

		remind_data_length -= data_length;
		if(set_clear_bitmap(p_object->p_bitmap, revised_offset, data_length, control_bit))
			return -1;

		if(remind_data_length <= 0)
		{
			p_bitmap->map_dirty = 1;
			return 0;
		}

		if(obj_bitmap_write(p_object, p_bitmap->cur_index, p_bitmap->p_map))
			return -1;			

		if(p_bitmap->cur_index == p_bitmap->cur_total_bitmap - 1)
		{
			p_bitmap->cur_index++;
			p_bitmap->cur_total_bitmap++;
			
			memset(p_bitmap->p_map, 0, p_bitmap->size_b_map_per_cluster);			
			if(set_clear_bitmap(p_object->p_bitmap, 
								0, 
								ALIGN_16B(p_bitmap->size_b_map_per_cluster, p_bitmap->bit_per_byte), 
								set_bit))
				return -1;	
		}
		else 
		{	
			p_bitmap->cur_index++;
			obj_bitmap_read(p_object, p_bitmap->cur_index, p_bitmap->p_map);
		}

		revised_offset = p_bitmap->size_b_map_per_cluster;
		
		p_bitmap->map_dirty = 1;
	}

	return 0;
}

int obj_bitmap_set(struct OBJECT *p_object, offset_t offset, uint32 data_length)
{
	return obj_bitmap_sub_set_clear(p_object, offset, data_length, set_bit);
}

int obj_bitmap_clear(struct OBJECT *p_object, offset_t offset, uint32 data_length)
{
	return obj_bitmap_sub_set_clear(p_object, offset, data_length, clear_bit);
}

offset_t obj_bitmap_get_free_offset(struct OBJECT *p_object, uint32 data_length)
{
	char *p_read_bitmap = NULL;
	offset_t free_offset = 0;
	int bitmap_num = 0;
	struct BITMAP *p_bitmap = p_object->p_bitmap;
	uint32 size_b_map_per_cluster = p_bitmap->size_b_map_per_cluster;

	free_offset = get_free_offset(p_bitmap->p_map, size_b_map_per_cluster, data_length);			
	if(free_offset != 0)
	{
		return (p_bitmap->cur_index * fl_get_cluster_size()) + free_offset;
	}

	p_read_bitmap = (char *)malloc(size_b_map_per_cluster);
	if(p_read_bitmap == NULL)
	{
		printf("Error, Failed to alloc bitmap \r\n");
		goto error_out;
	}

	for(bitmap_num = p_bitmap->cur_total_bitmap - 1; bitmap_num >= 0 ; bitmap_num--)
	{
		if(bitmap_num == p_bitmap->cur_index)
			continue;

		memset(p_read_bitmap, 0, size_b_map_per_cluster);
		if(obj_bitmap_read(p_object, bitmap_num, p_read_bitmap))
		{
			printf("Error, read bitmap\r\n");
			goto error_out;
		}

		free_offset = get_free_offset(p_read_bitmap, size_b_map_per_cluster, data_length);			
		if(free_offset != 0)
		{
			if(p_bitmap->map_dirty == 1)
				obj_bitmap_write(p_object, p_bitmap->cur_index, p_bitmap->p_map);

			free(p_bitmap->p_map);
			p_bitmap->cur_index = bitmap_num;			
			p_bitmap->p_map = p_read_bitmap;
			p_bitmap->map_dirty = 0;

			return (p_bitmap->cur_index * fl_get_cluster_size()) + free_offset;
		}
	}

	if(free_offset == 0)
	{
		if(p_bitmap->map_dirty == 1)
			obj_bitmap_write(p_object, p_bitmap->cur_index, p_bitmap->p_map);
		
		if(p_bitmap->p_map != NULL)
			free(p_bitmap->p_map);

		memset(p_read_bitmap, 0, p_bitmap->size_b_map_per_cluster);
		p_bitmap->p_map = p_read_bitmap;
				
		if(set_clear_bitmap(p_bitmap, 0, p_bitmap->size_b_map_per_cluster, set_bit))
			goto error_out;

		free_offset = get_free_offset(p_read_bitmap, size_b_map_per_cluster, data_length);
		if(free_offset == 0)
			goto error_out;	
				
		p_bitmap->cur_index = p_bitmap->cur_total_bitmap;		
		p_bitmap->cur_total_bitmap++;
		return (p_bitmap->cur_index * fl_get_cluster_size()) + free_offset;	
	}

error_out:
	if(p_read_bitmap != NULL)
		free(p_read_bitmap);

	return 0;
}

static int sub_get_free_offset(char *p_map, const uint32 size_b_map, uint32 data_length, offset_t *offset)
{
	uint32 need_bit_num = 0;
	uint32 continued_bit_cnt = 0;
	int i = 0, j = 0;
	int save_i, save_j = 0;
	const uint32 byte_max_offset = BIT_PER_BYTE - 1;

	need_bit_num = data_length / BYTE_PER_BIT;
	if (data_length % BYTE_PER_BIT > 0)
		need_bit_num++;

	// TBD : bit ????À» ???? free area Ã£?? ?Ë°í¸®??À» ?? È¿À²??À¸?? ?????? ?? ??À» ?? ????...
	for (i = 0; i < size_b_map; i++)
	{
		for (j = byte_max_offset; j >= 0; j--)
		{
			if (IS_SET_BIT(p_map[i], j) == 0)
			{
				if (continued_bit_cnt == 0)
				{
					save_i = i;
					save_j = j;
				}

				continued_bit_cnt++;
				if (continued_bit_cnt == need_bit_num)
				{
					*offset = ((save_i * BIT_PER_BYTE) + ((save_j - byte_max_offset) * (-1))) * BYTE_PER_BIT;
					return 1;
				}

			}
			else
				continued_bit_cnt = 0;
		}
	}

	if (continued_bit_cnt != 0)
		*offset = ((save_i * BIT_PER_BYTE) + ((save_j - byte_max_offset) * (-1))) * BYTE_PER_BIT;
	else
		*offset = NOT_INVALID_OFFSET;

	return 0;
}

// ???? Á¸??
// ????À» ?Æ·??? ????. 
// 4096byte ?Ì»??? free ????À» ????
// bitmap ?? free offsetÀ» ????À» ????È­ 
offset_t get_free_offset_extend(struct OBJECT *p_object, uint32 data_length)
{
	offset_t save_offset = 0;
	offset_t offset = 0;
	offset_t sli_offset = 0;			//?Ê¿? Á¶?Ç¿? ???????? ?Ê´?, ?????? bitmap???? ?????? offset, 
	offset_t bitmap_end_offset = 0;

	int ret = 0;
	uint32 remind_data_length = 0;
	uint32 i = 0;
	uint32 cluster_size_b = fl_get_cluster_size();
	struct BITMAP *p_bitmap = p_object->p_bitmap;

	remind_data_length = data_length;
	bitmap_end_offset = ALIGN_16B(p_bitmap->size_b_map_per_cluster, p_bitmap->bit_per_byte);

	for (i = 0; i < p_bitmap->cur_total_bitmap; i++)
	{
		ret = sub_get_free_offset(p_bitmap->p_map, p_bitmap->size_b_map_per_cluster, remind_data_length, &offset);
		if (ret == 1)
		{
			if (save_offset != 0)
				return save_offset;

			return ((p_bitmap->cur_index) * cluster_size_b) + offset;
		}

		if (offset == NOT_INVALID_OFFSET)
		{
			remind_data_length = data_length;
			save_offset = 0;
			offset = 0;
		}
		else if (offset == bitmap_end_offset)
		{
			if (save_offset == 0)
				save_offset = ((p_bitmap->cur_index) * cluster_size_b) + offset;

			remind_data_length = remind_data_length - (cluster_size_b - offset);
			if (remind_data_length == 0)
				return 0;

			if (i == p_bitmap->cur_total_bitmap - 1)
				i--;
		}
		else
		{
			remind_data_length = remind_data_length - (cluster_size_b - offset);
			if (remind_data_length == 0)
				return -1;

			save_offset = ((p_bitmap->cur_index) * cluster_size_b) + offset;
			if (i == p_bitmap->cur_total_bitmap - 1)
				i--;
		}

		if (p_bitmap->map_dirty == 1)
		{
			if (obj_bitmap_write(p_object, p_bitmap->cur_index, p_bitmap->p_map) < 0)
				return 0;
		}

		if (p_bitmap->cur_index == p_bitmap->cur_total_bitmap - 1)
		{
			if (offset > 0 && offset < cluster_size_b)
				sli_offset = ((p_bitmap->cur_index) * cluster_size_b) + offset;

			p_bitmap->cur_index = 0;
		}
		else
		{
			p_bitmap->cur_index++;
		}

		if (obj_bitmap_read(p_object, p_bitmap->cur_index, p_bitmap->p_map) < 0)
			return 0;

		p_bitmap->map_dirty = 0;
	}

	if (sli_offset != 0)
		return sli_offset;

	return ((p_bitmap->cur_index + 1) * cluster_size_b) + bitmap_end_offset;
}
