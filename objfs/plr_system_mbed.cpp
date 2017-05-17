#include <mbed.h>
#include "SDFileSystem.h"
#include <stdarg.h>
#include <stdio.h>
#include "plr_system_mbed.h"

#ifndef CONFIG_SYS_CBSIZE
#define CONFIG_SYS_CBSIZE 256
#endif

#define STATIC_BUFF_SIZE_MAX	32 * 1024

#ifdef TARGET_NUCLEO_F446ZE
Serial debug_serial(PB_10, PB_11);    // serial2
SDFileSystem sd(PA_7, PA_6, PA_5, PA_4, "sd", NC, SDFileSystem::SWITCH_NONE, 1000000);
#elif TARGET_ARCH_MAX
Serial debug_serial(PA_9, PA_10);    // serial2
SDFileSystem sd(PC_3, PC_2, PB_10, PE_2, "sd", NC, SDFileSystem::SWITCH_NONE, 30000000);
#endif

Timer timer;

char console_buffer[CONFIG_SYS_CBSIZE + 1];

//#ifdef __cplusplus
//extern "C" {
//#endif

int mbed_printf(const char *fmt, ...)
{
	// Parts of this are copied from mbed RawSerial ;)
    std::va_list arg;
    va_start(arg, fmt);
    
    int length= vsnprintf(NULL, 0, fmt, arg);
    char *temp = new char[length + 1];
    if(temp == NULL)
        return 0;   // I can't work like this!    

    vsprintf(temp, fmt, arg);
    debug_serial.puts(temp);
    delete[] temp;
    
    va_end(arg);
    return length;
}

static int readline_into_buffer (char * buffer)
{
    char *p = buffer;
    char *p_buf = buffer;
    int n = 0;              /* buffer index     */
    char c;
#ifdef PLR_SYSTEM   //160126 retry 및 err처리추가
    int retry = 10;
#endif

    for (;;) {
        if (debug_serial.readable()) {
            c = debug_serial.getc();
            //c = UART2->D;
            //debug_serial.printf ("! %c(%d)\n", c, c);
        }
        else
            continue;
#ifdef PLR_SYSTEM   //160126 retry 및 err처리 추가
        if (c == (char)-1){
            if (retry-- > 0)
                continue;
            else
                return -1;
        }
        else
            retry = 5;
#endif

        /*
         * Special character handling
         */
        switch (c) {
        case '\r':              /* Enter        */
        case '\n':
            *p = '\0';
            debug_serial.puts ("\r\n");
            return (p - p_buf);

        case '\0':              /* nul          */
            continue;

        case 0x03:              /* ^C - break       */
            p_buf[0] = '\0';    /* discard input */
            return (-1);

        default:
            /*
             * Must be a normal character then
             */
            if (n < CONFIG_SYS_CBSIZE-2) {
                debug_serial.putc (c);
                *p++ = c;
                ++n;
            }
        }
    }
}

int readline (const char *const prompt)
{
    int ret = 0;
    console_buffer[0] = '\0';
    
    ret =  readline_into_buffer(console_buffer);
    return ret;
}

unsigned int crc32 (unsigned int crc, const unsigned char *buf, unsigned int len)
{
	static unsigned int table[256];
	static int have_table = 0;
	unsigned int rem;
	uint8_t octet;
	int i, j;
	const unsigned char *p, *q;
 
	/* This check is not thread safe; there is no mutex. */
	if (have_table == 0) {
		/* Calculate CRC table. */
		for (i = 0; i < 256; i++) {
			rem = i;  /* remainder from polynomial division */
			for (j = 0; j < 8; j++) {
				if (rem & 1) {
					rem >>= 1;
					rem ^= 0xedb88320;
				} else
					rem >>= 1;
			}
			table[i] = rem;
		}
		have_table = 1;
	}
 
	crc = ~crc;
	q = buf + len;
	for (p = buf; p < q; p++) {
		octet = *p;  /* Cast to unsigned octet. */
		crc = (crc >> 8) ^ table[(crc & 0xff) ^ octet];
	}
	return ~crc;
}

void* get_read_buffer_addr(void)
{
    static char *read_buf = NULL;
    
    if (read_buf == NULL) {
        read_buf = (char*)malloc(sizeof(char) * STATIC_BUFF_SIZE_MAX);
		memset(read_buf, 0x0, sizeof(char) * STATIC_BUFF_SIZE_MAX);
    }
    
    return (void *)read_buf;    
}

void* get_write_buffer_addr(void)
{
    static char *write_buf = NULL;
    
    if (write_buf == NULL) {
        write_buf = (char*)malloc(sizeof(char) * STATIC_BUFF_SIZE_MAX);
		memset(write_buf, 0x0, sizeof(char) * STATIC_BUFF_SIZE_MAX);
    }
        
    return (void *)write_buf;
}   

void* get_extra_buffer_addr(void)
{
	static char *extra_buf;

	if (extra_buf == NULL) {
        extra_buf = (char*)malloc(sizeof(char) * STATIC_BUFF_SIZE_MAX);
		memset(extra_buf, 0x0, sizeof(char) * STATIC_BUFF_SIZE_MAX);
	}
	
    return (void*)extra_buf;
}

unsigned int get_max_buff_size(void)
{
	return STATIC_BUFF_SIZE_MAX;
}

void udelay(unsigned long usec)
{
    wait_us(usec);
}

int run_command (const char *cmd, int flag)
{
    return 0;
}

int console_init_r(void)
{
    debug_serial.baud(115200);
	
    return 0;
}

int tstc(void)
{
    return debug_serial.readable();
}

int mbed_getc(void)
{
    return debug_serial.getc();
}

int mbed_sd_initialize(void)
{
	int ret = 0;

	console_init_r();
	dbg_print("\nSD MMC initialize... ");
	ret = sd.disk_initialize();
	
    return ret;
}

int mbed_sd_read(unsigned char *data, unsigned int start_sector, unsigned int len)
{
    return sd.disk_read((uint8_t*)data, start_sector, len);
}

int mbed_sd_write(unsigned char *data, unsigned int start_sector, unsigned int len)
{
    return sd.disk_write((uint8_t*)data, start_sector, len);
}

int mbed_sd_erase(unsigned int start_sector, unsigned int len)
{
    return sd.disk_erase(start_sector, len);
}

int mbed_sd_total_block_count(void)
{
    return sd.disk_sectors();
}

void mbed_timer_reset(void)
{
	timer.stop();
	timer.reset();
	timer.start();
}

int mbed_get_timer_sec(void)
{
    //return timer.read();
    time_t seconds = time(NULL);

	return (int)seconds;
}

int mbed_get_timer_us(void)
{
    return timer.read_us();
}

void mbed_board_reset(void)
{
	NVIC_SystemReset();
}

void mbed_print_date_time(void)
{
	time_t seconds = time(NULL);
	debug_serial.printf ("%s\n", ctime(&seconds));
}

void mbed_set_date_time(unsigned char *date_info)
{
	struct tm t;
	char date_unit[2] = {0,};

	memcpy((void*)date_unit, &date_info[0], 2);
	t.tm_mon = atoi(date_unit);
	
	memcpy((void*)date_unit, &date_info[2], 2);
	t.tm_mday = atoi(date_unit);
	
	memcpy((void*)date_unit, &date_info[4], 2);
	t.tm_hour = atoi(date_unit);
	
	memcpy((void*)date_unit, &date_info[6], 2);
	t.tm_min = atoi(date_unit);
	
	memcpy((void*)date_unit, &date_info[8], 2);
	t.tm_year = atoi(date_unit);
	
	memcpy((void*)date_unit, &date_info[11], 2);
	t.tm_sec = atoi(date_unit);
	
	t.tm_year = t.tm_year + 2000 - 1900;
	t.tm_mon = t.tm_mon - 1;

	set_time(mktime(&t));
}

void mbed_get_product_name(char *name)
{
	struct SDFileSystem::mmc_cid *cid;
	cid = sd.get_cid();

	sprintf ((char*)name, "%s_%u", cid->prod_name, cid->serial);	
}

//#ifdef __cplusplus
//}
//#endif