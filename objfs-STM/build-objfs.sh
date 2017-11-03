#!/bin/bash

TARGET=objfs
TOPDIR=`pwd`
BUILD="$TOPDIR/BUILD"
MBEDOS="$TOPDIR/mbed-os-program"

SRC1="$TOPDIR/src"
echo $SRC1
find $SRC1 -type f\
    \( -name '*.o' -o -name '*.su' \) -print \
    | xargs rm -f

rm -v ./$TARGET.bin

cd $TOPDIR/mbed-os-program/

mbed config --local GCC_ARM_PATH $TOPDIR/gcc/gcc-arm-none-eabi-6-2017-q2-update/bin/
mbed config --local TARGET NUCLEO_F446ZE
mbed config --local TOOLCHANIN GCC_ARM

if [ "$1" = "c" ] || [ "$1" = "clean" ]
then
    mbed compile --clean --source $SRC1 --source $MBEDOS -t GCC_ARM -D CONFIG_MBED -N $TARGET --build $BUILD
    cd $TOPDIR
    ./mktags.sh
else
    mbed compile --source $SRC1 --source $MBEDOS -t GCC_ARM -D CONFIG_MBED -N $TARGET --build $BUILD
fi


cd $TOPDIR
cp $TOPDIR/BUILD/$TARGET.bin $TOPDIR
