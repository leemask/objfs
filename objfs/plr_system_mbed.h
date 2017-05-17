#ifndef __PLR_SYSTEM_MBED_H__
#define __PLR_SYSTEM_MBED_H__
#include <mbed.h>

int mbed_printf(const char *fmt, ...);

int console_init_r(void);
int readline (const char *const prompt);
int run_command (const char *cmd, int flag);
unsigned int crc32 (unsigned int crc, const unsigned char *buf, unsigned int len);

void* get_read_buffer_addr(void);
void* get_write_buffer_addr(void);
void* get_extra_buffer_addr(void);
unsigned int get_max_buff_size(void);

void udelay(unsigned long usec);
int tstc(void);
int mbed_getc(void);

int mbed_sd_initialize(void);
int mbed_sd_read(unsigned char *data, unsigned int start_sector, unsigned int len);
int mbed_sd_write(unsigned char *data, unsigned int start_sector, unsigned int len);
int mbed_sd_total_block_count(void);

void mbed_timer_reset(void);
int mbed_get_timer_sec(void);
int mbed_get_timer_us(void);

void mbed_board_reset(void);

void mbed_set_date_time(unsigned char *date_info);
void mbed_print_date_time(void);
void mbed_get_product_name(char *name);
#define dbg_print debug_serial.printf



extern Serial debug_serial;


#endif
