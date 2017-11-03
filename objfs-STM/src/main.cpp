#include "mbed.h"

#include <stdio.h>
#include <memory.h>

#include "fat_filelib.h"
#include "virtual_media.h"

#include "object.h"
#include "object_stack.h"
#include "object_queue.h"
#include "object_stream.h"
#include "object_index.h"

#include "plr_system_mbed.h"

Serial pc(USBTX, USBRX);
DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
extern int	 test(int);

int main()
{
    int ret;
    pc.baud(115200);

    led1 = 1;
    led2 = 0;
    led3 = 0;

	// Media Init and File System Format
	//-----------------------------------------------------------------------------------
	// Initialise media
	dbg_print("Start!!--------\r\n");
	if(ret = media_init(1, "PRAM")) {
		pc.printf("media init fail(%d)\r\n", ret);
		return 1;
	}
    // Initialise File IO Library
    fl_init();

	// Attach media access functions to library
    if (fl_attach_media(media_read_sector, media_write_sector) != FAT_INIT_OK)
    {
        pc.printf("ERROR: Media attach failed\r\n");
        return 1;
    }

    led2 = 1;

	dbg_print("start format \r\n");
	fl_format(0x200000, "PRAM");
	wait(1);

    led3 = 1;

	pc.printf("test_basic start!!\r\n");
	test_basic();
	pc.printf("test_basic finish!!\r\n");

    fl_shutdown();

    while (1) {
        led1 = !led1;
        led2 = !led2;
        led3 = !led3;
        wait(1);
    }

	return 1;
}
