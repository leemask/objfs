//#include "mbed.h"

#include <stdio.h>
#include <memory.h>
#include <unistd.h>

#include "fat_filelib.h"
#include "virtual_media.h"

#include "object.h"
#include "object_stack.h"
#include "object_queue.h"
#include "object_stream.h"
#include "object_index.h"

//#include "plr_system_mbed.h"
#include "plr_system.h"
#include "plr_socket.h"

int main()
{
    int ret;

//	HttpClient client;

	// Media Init and File System Format
	//-----------------------------------------------------------------------------------
	// Initialise media
	dbg_print("Start!!--------\r\n");
	
#if 1
	if(ret = media_init(1, (char*)"PRAM")) {
		printf("media init fail(%d)\r\n", ret);
		return 1;
	}
    // Initialise File IO Library
    fl_init();

	// Attach media access functions to library
    if (fl_attach_media(media_read_sector, media_write_sector) != FAT_INIT_OK)
    {
        printf("ERROR: Media attach failed\r\n");
        return 1;
    }

	dbg_print("start format \r\n");
	fl_format(0x200000, (const char*)"PRAM");
	sleep(1);

	printf("test_basic start!!\r\n");
	test_basic();
	printf("test_basic finish!!\r\n");

    fl_shutdown();
/*
    while (1) {
		usleep(1000000);
    }
*/
#endif
	return 1;
}
