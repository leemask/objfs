#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <linux/fs.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <memory.h>
#include <plr_system.h>
#define SEC2BYTE(sec)	(((off64_t)sec)<<9)
#define BYTE2SEC(byte)	(u32)(byte>>9)

#ifndef O_DIRECT
#ifndef __O_DIRECT
#define O_DIRECT	0200000 /* Direct disk access.  */
#else
#define O_DIRECT 	__O_DIRECT
#endif
#endif

//#define SD_PATH		"/dev/sdb"
#define SD_PATH		"/dev/mmcblk1"

char *g_read_buf_addr;
char *g_write_buf_addr;
char *g_extra_buf_addr;

int mmc_fd;
int sd_state_fd;

int prepare_buffers(void)
{
	g_read_buf_addr = (char *)valloc(PAGE_ALIGN(READ_BUF_SIZE));
	if (g_read_buf_addr == NULL){
		dbg_print("read buff create fail\n");
		perror("");
		return -1;
	}

	g_write_buf_addr = (char *)valloc(PAGE_ALIGN(WRITE_BUF_SIZE));
	if (g_write_buf_addr == NULL){
		dbg_print("read buff create fail\n");
		perror("");
		return -1;
	}

	g_extra_buf_addr = (char *)valloc(PAGE_ALIGN(EXTRA_BUF_SIZE));
	if (g_extra_buf_addr == NULL){
		dbg_print("read buff create fail\n");
		perror("");
		return -1;
	}

	memset(g_read_buf_addr, 0, PAGE_ALIGN(READ_BUF_SIZE));
	memset(g_write_buf_addr, 0, PAGE_ALIGN(WRITE_BUF_SIZE));
	memset(g_extra_buf_addr, 0, PAGE_ALIGN(EXTRA_BUF_SIZE));
	//memset(g_mmc_cond_buf_addr, 0, sizeof(struct mmc_poweroff_cond));

	return 0;
}

void free_buffers(void)
{
	if (g_read_buf_addr)
		free(g_read_buf_addr);
	if (g_write_buf_addr)
		free(g_write_buf_addr);
	if (g_extra_buf_addr)
		free(g_extra_buf_addr);

	g_read_buf_addr = NULL;
	g_write_buf_addr = NULL;
	g_extra_buf_addr = NULL;
}

int prepare_sd(void)
{
	int fd;

	if (mmc_fd != 0){
		dbg_print("Already open path %d\n", mmc_fd);
		return 0;
	}

	//fd = open(SD_PATH, O_RDWR|O_LARGEFILE|O_DIRECT);
	fd = open(SD_PATH, O_RDWR|O_LARGEFILE);

	if (fd < 0){
		dbg_print("Test path open failed %s\n", SD_PATH);
		perror("");
		return -1;
	}

	mmc_fd = fd;

	dbg_print ("Test path : %s\n", SD_PATH);

	return 0;
}

int close_sd(void)
{
	if (!mmc_fd){
		//plr_err("Already close mmc\n");
		return 0;
	}

	if (close(mmc_fd)){
		dbg_print("mmc close fail : ");
		perror("");
		return -1;
	}
	mmc_fd = 0;

	return 0;
}

int sd_initialize(void)
{
	int ret = 0;

	//ret = prepare_buffers();

	//if (ret)
	//	return ret;
	
	ret = prepare_sd();
	
    return ret;
}

int sd_read(unsigned char *data, unsigned int start_sector, unsigned int len)
{
    ssize_t ret;
	off64_t ret_off;
	size_t b_len = (size_t)SEC2BYTE(len);

	ret_off = lseek64(mmc_fd, SEC2BYTE(start_sector), SEEK_SET);
	if (ret_off < 0){
		dbg_print("read lseek64 fail. fd : %d, sec : %u, to byte : %jd\n",
				mmc_fd, start_sector, SEC2BYTE(start_sector));
		perror("read lseek64 fail");
		return -1;
	}

	ret = read(mmc_fd, data, b_len);

	if (ret <= (ssize_t)b_len && ret >= 0)
		return 0;
	else
	{
		perror("read fail");
		return -1;
	}
}

int sd_write(unsigned char *data, unsigned int start_sector, unsigned int len)
{
    ssize_t ret;
	off64_t ret_off;
	size_t b_len = (size_t)SEC2BYTE(len);

	ret_off = lseek64(mmc_fd, SEC2BYTE(start_sector), SEEK_SET);
	if (ret_off < 0){
		dbg_print("write lseek64 fail. fd : %d, sec : %u, to byte : %jd\n",
				mmc_fd, start_sector, (off64_t)SEC2BYTE(start_sector));
		perror("write lseek64 fail");
		return -1;
	}

	ret = write(mmc_fd, data, b_len);

	if (ret == (ssize_t)b_len)
		return 0;
	else {
		perror("read fail");
		return -1;
	}
}

/*
int do_read(uint dev_num, uchar *data, uint start_sector, uint len)
{
	ssize_t ret;
	off64_t ret_off;
	size_t b_len = (size_t)SEC2BYTE(len);

	ret_off = lseek64(dev_num, SEC2BYTE(start_sector), SEEK_SET);
	if (ret_off < 0){
		plr_err("read lseek64 fail. fd : %d, sec : %u, to byte : %jd\n",
				dev_num, start_sector, SEC2BYTE(start_sector));
		perror("read lseek64 fail");
		return -1;
	}

	ret = read(dev_num, data, b_len);

	if (ret <= (ssize_t)b_len && ret >= 0)
		return 0;
	else
		return -1;
}

int do_write(uint dev_num, uchar *data, uint start_sector, uint len)
{
	ssize_t ret;
	off64_t ret_off;
	size_t b_len = (size_t)SEC2BYTE(len);

	ret_off = lseek64(dev_num, SEC2BYTE(start_sector), SEEK_SET);
	if (ret_off < 0){
		plr_err("write lseek64 fail. fd : %d, sec : %u, to byte : %jd\n",
				dev_num, start_sector, (off64_t)SEC2BYTE(start_sector));
		perror("write lseek64 fail");
		return -1;
	}

	ret = write(dev_num, data, b_len);

	if (ret == (ssize_t)b_len)
		return 0;
	else {
		return -1;
	}
}
*/
