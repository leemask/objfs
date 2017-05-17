#include "mbed.h"

#include <stdio.h>
#include <memory.h>

#include "fat_filelib.h"
#include "virtual_media.h"

#include "object.h"
#include "object_stack.h"
#include "object_queue.h"
#include "object_stream.h"
#include "Object_index.h"

#include "plr_system_mbed.h"


//#include "rtos.h”
//#include "rtos.h"


DigitalOut led1(LED1);
extern int	 test(int);
#if 0
int main()
{
	int sum, first =1;
    while (1) {
        led1 = !led1;
         wait(1);
        //Thread::wait(500);
	printf("-----\r\n"); 
    }
}
#else
int main()
{
	// Media Init and File System Format
	//-----------------------------------------------------------------------------------
	// Initialise media
	dbg_print("Start--------\n");
	if(media_init(1, "PRAM"))
		return 1;	
	
	// make MBR and FAT32 filesystem in media.
	//media_format_for_fatfs();

    // Initialise File IO Library
    fl_init();
	
	// Attach media access functions to library
    if (fl_attach_media(media_read_sector, media_write_sector) != FAT_INIT_OK)
    {
        printf("ERROR: Media attach failed\n");
        return 1; 
    }

	// Attach media access functions to library
	// ieora@elixirflash.com
//	if (fl_attach_media_byte_io(media_read_byte, media_write_byte) != FAT_INIT_OK)
 //   {
 //       printf("ERROR: Media attach failed\n");
 //       return 1; 
 //   }

	//Format media for object dir and testing. 
	dbg_print("start format \n");
//	fl_format(0x200000, "PRAM");


	//-----------------------------------------------------------------------------------
	test_basic();
return 1;	
	test_routine_obj_stack();

	test_routine_obj_queue();
	test_routine_obj_stream();
	test_routine_obj_index();
	//-----------------------------------------------------------------------------------

    fl_shutdown();

	getchar();
	return 1;
}



#endif
