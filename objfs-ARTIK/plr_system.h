#ifndef _PLR_SYSTEM_H_
#define _PLR_SYSTEM_H_

#define WRITE_BUF_SIZE                0x01000000
#define READ_BUF_SIZE                 0x00800000
#define EXTRA_BUF_SIZE                0x00800000

#ifndef PAGE_SIZE
#define PAGE_SIZE               4096
#endif

#define __ALIGN_MASK(x, mask)   (((x) + mask) & ~(mask))
#define PAGE_ALIGN(addr)        ALIGN(addr, PAGE_SIZE)
#define ALIGN(x, a)             __ALIGN_MASK(x, (typeof(x))(a) - 1)

#define dbg_print(fmt, args...) printf(fmt, ##args)
int sd_initialize(void);
int sd_read(unsigned char *data, unsigned int start_sector, unsigned int len);
int sd_write(unsigned char *data, unsigned int start_sector, unsigned int len);
#endif