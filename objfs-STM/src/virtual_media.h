#ifndef __VIRTUAL_MEDIA__
#define __VIRTUAL_MEDIA__

#define MAX_MEDIA_SIZE_GB	2
#define MIN_MEDIA_SIZE_GB	1
#define SECTOR_SIZE_B		512
#define SIZE_B_CLUSTER		4096

typedef struct _MEDIA
{
	char str_media_name[32];
	char *media;
	unsigned int media_size_gb;
}MEDIA;

int media_init(unsigned int media_size_gb, char *str_media_type);

int media_read_sector(unsigned long sector, unsigned char *buffer, unsigned long sector_count);
int media_write_sector(unsigned long sector, unsigned char *buffer, unsigned long sector_count);

int media_read_byte(unsigned long addr, unsigned char *buffer, unsigned long length);
int media_write_byte(unsigned long addr, unsigned char *buffer, unsigned long length);

int media_format_for_fatfs(void);
#endif