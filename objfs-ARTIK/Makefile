TEST_TARGET = plrtest

#CROSS = /opt/arm-2009q3/bin/arm-none-linux-gnueabi-

CC 	= $(CROSS)g++
AR 	= $(CROSS)ar
LD 	= $(CROSS)ld
NM 	= $(CROSS)nm
RANLIB 	= $(CROSS)ranlib
STRIP = $(CROSS)strip
#INCLUDE = -I. #-I$(INCDIR)
INCLUDE = -I. -I ./include

#CFLAGS = -Wall -g -Werror -Wno-error=unused-variable -Wno-error=unused-function -Wno-error=unused-but-set-variable -Wno-error=int-to-pointer-cast
CFLAGS = -g
CFLAGS += -D_LARGEFILE64_SOURCE
#CFLAGS += -D_FILESYSTEM_TEST_ -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -DCONFIG_EMBEDDED_KERNEL -DCONFIG_TARGET_LINUX_NETWORK #-DPLRTEST_SSD_FOR_MICRON

#LDFLAGS = -L$(LIBDIR) -lc -lm -ldl -L. -lpthread -lrt $(INCLUDE)

# sources of plrtest
SRC += $(wildcard *.cpp)
#SRC += $(PLRTEST_PATH)/test_case/plr_ddxx0004.c

OBJS = $(SRC:.cpp=.o)

# define the rule
.SUFFIXES:.cpp .o

.cpp.o:
	@echo Compiling: $<
	$(CC) -c $(CFLAGS)  $(INCLUDE) -o $@ $<

all: app

app: $(OBJS)
	@echo "\nLinking: $(TEST_TARGET) \n"
	$(CC) -o $(TEST_TARGET) $(OBJS) $(PRELINK) $(POSTLINK) $(LDFLAGS) 

clean:
	rm -vf $(OBJS)  $(TEST_TARGET)

