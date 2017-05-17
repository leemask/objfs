#!/bin/sh

SRC1="/mnt/share/objfs"
find $SRC1 -type f \
			 \( -name '*.o' -o -name '*.su' \) -print \
			 | xargs rm -f


#mbed config --global GCC_ARM_PATH /opt/gcc-arm-none-eabi-4_9-2015q1/bin
#mbed config --global TARGET NUCLEO_F446ZE
#mbed config --global TOOLCHANIN GCC_ARM

#mbed config --local GCC_ARM_PATH /opt/gcc-arm-none-eabi-4_9-2015q1/bin
mbed config --local TARGET NUCLEO_F446ZE
#mbed config --local TARGET ARCH_MAX
mbed config --local TOOLCHANIN GCC_ARM

if [ "$1" = "c" ] || [ "$1" = "clean" ]
then
mbed compile --clean --source $SRC1 --source . -t GCC_ARM -D CONFIG_MBED
else
mbed compile --source $SRC1 --source . -t GCC_ARM -D CONFIG_MBED
fi
cp ./BUILD/NUCLEO_F446ZE/GCC_ARM/objfs.bin /media/andrew/NODE_F446ZE/
