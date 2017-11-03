#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

#include "virtual_media.h"
#include "fat_filelib.h"

#define MBED_SD
#ifdef MBED_SD//mbed SD
//#include <mbed.h>
//#include "SDFileSystem.h"
#include <plr_system.h>
#include <stdarg.h>
#include <stdio.h>


#if 0
#ifdef TARGET_NUCLEO_F446ZE

Serial debug_serial(PB_10, PB_11);    // serial2
SDFileSystem sd(PA_7, PA_6, PA_5, PA_4, "sd", NC, SDFileSystem::SWITCH_NONE, 1000000);
#elif TARGET_ARCH_MAX
Serial debug_serial(PA_9, PA_10);    // serial2
SDFileSystem sd(PC_3, PC_2, PB_10, PE_2, "sd", NC, SDFileSystem::SWITCH_NONE, 30000000);
#endif
#endif

//extern int mbed_sd_initialize(void);

//extern SDFileSystem sd;

int media_init(unsigned int media_size_gb, char *str_media_type) 
{
	int ret = 0;

	//ret = mbed_sd_initialize();
	ret = sd_initialize();

	return ret;

#if 0
	MEDIA *p_media = NULL;
	int i = 0;

	if(media_size_gb < MIN_MEDIA_SIZE_GB || media_size_gb > MAX_MEDIA_SIZE_GB)
	{
		printf("Error. Media Size need to be smaller than %d or greater than %d. \n"
			,MAX_MEDIA_SIZE_GB
			, MIN_MEDIA_SIZE_GB);

		return -1;
	}

	p_media = (MEDIA *)malloc(sizeof(MEDIA));
	if(p_media == NULL)
	{
		printf("Error. Failed to crate a media \n");
		return -1;	
	}

	p_media->media_size_gb = media_size_gb;
	p_media->media = (char *)malloc(p_media->media_size_gb * 1024 * 1024 * 1024);
	if(p_media->media == NULL)
	{
		printf("Error. Failed to allocate a memory to store a data. \n");
		return -1;
	}
	
	
	memset(p_media->media, 0, p_media->media_size_gb * 1024 * 1024 * 1024);	
	memcpy(p_media->str_media_name, str_media_type, 32);	
	
	_p_media = p_media;
	
	printf("media address 0x%08X \n", _p_media->media);
	return 0;

#endif

}


static void make_MBR(void)
{
	unsigned char buffer[SECTOR_SIZE_B];
	
	memset(buffer, 0, SECTOR_SIZE_B);

	/*Bootable Flag*/
	buffer[446] = 0x00;

	/*Starting CHA Address*/
	buffer[447] = 0x00;
	buffer[448] = 0x01;
	buffer[449] = 0x01;
	
	/*Partition Type*/
	buffer[450] = 0x0C;
	
	/*Ending CHA Address*/
	buffer[451] = 0xFE;
	buffer[452] = 0xFF;
	buffer[453] = 0xFF;	
	
	/*Start LBA Address*/
	buffer[454] = 0x3F;
	buffer[455] = 0x00;
	buffer[456] = 0x00;
	buffer[457] = 0x00;	

	/*Size in Sector*/
	buffer[458] = 0x00;
	buffer[459] = 0x00;
	buffer[460] = 0x20;
	buffer[461] = 0x00;

	/*MBR Signature*/
	buffer[510] = 0x55;
	buffer[511] = 0xAA;

	media_write_sector(0, buffer, 1);
}

static int make_fatfs_boot_record_for_fat32(unsigned int boot_sector_lba, unsigned int vol_sectors, const char *name)
{		
	unsigned char buffer[SECTOR_SIZE_B];

	unsigned int i;
    unsigned int total_clusters;
	unsigned int fat_sectors = 0;
	unsigned int total_cluster = 0;
	const unsigned int sectors_per_cluster = SIZE_B_CLUSTER / SECTOR_SIZE_B;		//4KB
	const unsigned int rootdir_first_cluster = 2;
	const unsigned int fs_info_sector = 1;

    // Zero sector initially
    memset(buffer, 0, FAT_SECTOR_SIZE);

    // OEM Name & Jump Code
    buffer[0] = 0xEB;
    buffer[1] = 0x3C;
    buffer[2] = 0x90;
    buffer[3] = 0x4D;
    buffer[4] = 0x53;
    buffer[5] = 0x44;
    buffer[6] = 0x4F;
    buffer[7] = 0x53;
    buffer[8] = 0x35;
    buffer[9] = 0x2E;
    buffer[10] = 0x30;

    // Bytes per sector
    buffer[11] = (FAT_SECTOR_SIZE >> 0) & 0xFF;
    buffer[12] = (FAT_SECTOR_SIZE >> 8) & 0xFF;
	    
    // Sectors per cluster
	buffer[13] = sectors_per_cluster;

    // Reserved Sectors
    buffer[14] = (32 >> 0) & 0xFF;
    buffer[15] = (32 >> 8) & 0xFF;

    // Number of FATS
    buffer[16] = 0x02;

    // Max entries in root dir (FAT16 only)
    buffer[17] = 0;
    buffer[18] = 0;
    
    // [FAT16] Total sectors (use FAT32 count instead)
    buffer[19] = 0x00;
    buffer[20] = 0x00;

    // Media type
    buffer[21] = 0xF8;

    // FAT32 BS Details
    // Count of sectors used by the FAT table (FAT16 only)
    buffer[22] = 0;
    buffer[23] = 0;

    // Sectors per track (default)
    buffer[24] = 0x3F;
    buffer[25] = 0x00;

    // Heads (default)
    buffer[26] = 0xFF;
    buffer[27] = 0x00;

    // Hidden sectors
    buffer[28] = 0x00;
    buffer[29] = 0x00;
    buffer[30] = 0x00;
    buffer[31] = 0x00;

    // Total sectors for this volume
    buffer[32] = (uint8)((vol_sectors>>0)&0xFF);
    buffer[33] = (uint8)((vol_sectors>>8)&0xFF);
    buffer[34] = (uint8)((vol_sectors>>16)&0xFF);
    buffer[35] = (uint8)((vol_sectors>>24)&0xFF);

    total_clusters = (vol_sectors / sectors_per_cluster) + 1;
    fat_sectors = (total_clusters / (SECTOR_SIZE_B / 4)) + 1;

    // BPB_FATSz32
    buffer[36] = (uint8)((fat_sectors>>0)&0xFF);
    buffer[37] = (uint8)((fat_sectors>>8)&0xFF);
    buffer[38] = (uint8)((fat_sectors>>16)&0xFF);
    buffer[39] = (uint8)((fat_sectors>>24)&0xFF);

    // BPB_ExtFlags
    buffer[40] = 0;
    buffer[41] = 0;

    // BPB_FSVer
    buffer[42] = 0;
    buffer[43] = 0;

    // BPB_RootClus
    buffer[44] = (uint8)((rootdir_first_cluster>>0)&0xFF);
    buffer[45] = (uint8)((rootdir_first_cluster>>8)&0xFF);
    buffer[46] = (uint8)((rootdir_first_cluster>>16)&0xFF);
    buffer[47] = (uint8)((rootdir_first_cluster>>24)&0xFF);

    // BPB_FSInfo
    buffer[48] = (uint8)((fs_info_sector>>0)&0xFF);
    buffer[49] = (uint8)((fs_info_sector>>8)&0xFF);

    // BPB_BkBootSec
    buffer[50] = 6;
    buffer[51] = 0;

    // Drive number
    buffer[64] = 0x00;

    // Boot signature
    buffer[66] = 0x29;

    // Volume ID
    buffer[67] = 0x12;
    buffer[68] = 0x34;
    buffer[69] = 0x56;
    buffer[70] = 0x78;

    // Volume name
    for (i=0;i<11;i++)
    {
        if (i < (int)strlen(name))
            buffer[i+71] = name[i];
        else
            buffer[i+71] = ' ';
    }

    // File sys type
    buffer[82] = 'F';
    buffer[83] = 'A';
    buffer[84] = 'T';
    buffer[85] = '3';
    buffer[86] = '2';
    buffer[87] = ' ';
    buffer[88] = ' ';
    buffer[89] = ' ';

    // Signature
    buffer[510] = 0x55;
    buffer[511] = 0xAA;
    
    if (media_write_sector(boot_sector_lba, buffer, 1))
        return 1;
    else
        return 0;
}

int media_format_for_fatfs(void)
{
	/*init MBR area*/
	make_MBR();
	
	/*init FAT32*/
	make_fatfs_boot_record_for_fat32(63, 0x200000, "PRAM");
}

//int media_read_sector(unsigned long sector, unsigned char *buffer, unsigned long sector_count)
int media_read_sector(uint32 sector, uint8 *buffer, uint32 sector_count)
{
#ifdef MBED_SD
	int ret = sd_read(buffer, sector, sector_count);

	if(ret)
		printf("temp log disk_read error %d\r\n", ret);

	return (ret ==0) ? 1 : 0;
#else
    unsigned long i, j;
	char *target_addr;

    for (i = 0; i < sector_count; i++)
    {
		target_addr = _p_media->media + (sector * SECTOR_SIZE_B);
		for(j = 0; j < SECTOR_SIZE_B; j++)
		{
			buffer[j] = target_addr[j];
		}

        sector ++;
        buffer += 512;
    }

    return 1;
#endif

}

//int media_write_sector(unsigned long sector, unsigned char *buffer, unsigned long sector_count)
int media_write_sector(uint32 sector, uint8 *buffer, uint32 sector_count)
{
#ifdef MBED_SD
	int ret = sd_write(buffer, sector, sector_count);	

	if(ret)
		printf("temp log disk_write error %d\r\n", ret);

	return (ret ==0) ? 1 : 0;

#else

    unsigned long i, j;
	char *target_addr;

    for (i = 0; i < sector_count; i++)
    {   
		target_addr = _p_media->media + (sector * SECTOR_SIZE_B);
		for(j = 0; j < SECTOR_SIZE_B; j++)
		{
			target_addr[j] = buffer[j];
		}

        sector ++;
        buffer += 512;
    }

	return 1;
#endif
}

#ifndef MBED_SD

int media_write_byte(unsigned long addr, unsigned char *buffer, unsigned long length)
{

#ifdef MBED_SD
	
#else

    unsigned long i = 0;
	char *target_addr = _p_media->media + addr;

	for (i = 0; i < length; i++)
    {   
		target_addr[i] = buffer[i];
    }

	return 1;
#endif
}

int media_read_byte(unsigned long addr, unsigned char *buffer, unsigned long length)
{
#ifdef MBED_SD
	
#else

    unsigned long i = 0;
	char *target_addr = _p_media->media + addr;

    for (i = 0; i < length; i++)
    {
		buffer[i] = target_addr[i];		
    }

    return 1;
#endif
}
#endif
#endif
