MYBIN = linklist_dm
MYBIN_FILES += $(wildcard *.c)

#CC = mips-openwrt-linux-gcc
CC=gcc

TARGET = $(MYBIN)

all:clean $(TARGET)

MYBIN_OBJS = $(MYBIN_FILES:.c=.o)


$(MYBIN): $(MYBIN_OBJS)
	$(CC) -Wall -o $@ $^

clean:
	rm -f *.o $(MYBIN) 
