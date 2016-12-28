AR = mipsel-openwrt-linux-uclibc-ar

TARGET_STATIC = libppc.a
TARGET_SHARE = libppc.so
TARGET_TEST = libppc_test

TARGET = $(TARGET_TEST) $(TARGET_STATIC) $(TARGET_SHARE)

TEST_FILES = test_main.c

SRC_TOOLS_PATH = ./tools
SOURCE_FILES = $(wildcard $(SRC_TOOLS_PATH)/*.c)

SRC_FILE_PATH = ./file_task
SOURCE_FILES += $(wildcard $(SRC_FILE_PATH)/*.c)

SRC_PPCLIB_PATH = ./ppclib
SOURCE_FILES += $(wildcard $(SRC_PPCLIB_PATH)/*.c)

TEST_FILES += $(SOURCE_FILES)

LIB_STATIC_FILES_OBJS = $(patsubst %.c,%.o,$(SOURCE_FILES))

all: $(TARGET)
$(TARGET_TEST): $(TEST_FILES)
	$(CC) $(CFLAGS) $(LDFLAGS)  $(TEST_FILES) -ljson-c -o  $(TARGET_TEST)

$(TARGET_STATIC): $(LIB_STATIC_FILES_OBJS)
	$(AR) -cqs $@ $^

$(TARGET_SHARE): $(SOURCE_FILES)
	$(CC) $(CFLAGS) $(LDFLAGS) -Wall -O2 -fPIC -shared -o $(TARGET_SHARE)
	
.PHONY: clean

clean:
	rm -f *.o $(TARGET)
