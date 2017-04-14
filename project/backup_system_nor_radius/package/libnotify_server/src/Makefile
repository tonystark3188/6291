TARGET = $(TARGET_SERVER) $(TARGET_LIB)
TARGET_SERVER = notify_server_test
TARGET_LIB = libnotify_server.so
OBJS_SERVER = test_main.c
OBJS_LIB = notify_server.c
SRC_MSGLIB_PATH := ./msg_lib
OBJS_LIB += $(wildcard $(SRC_MSGLIB_PATH)/*.c)

all: $(TARGET) install

$(TARGET_LIB):$(OBJS_LIB)
	$(CC) $(CFLAGS) $(LDFLAGS) -Wall -O2 -fPIC -shared -ljson-c -pthread -o $@ $^ 
	
$(TARGET_SERVER): $(OBJS_LIB) $(OBJS_SERVER)
	$(CC) $(CFLAGS) $(LDFLAGS) -ljson-c -pthread -o $@ $^ 
	
install:
	mkdir lib
	mkdir bin
	cp -f $(TARGET_LIB) lib/
	cp -f $(TARGET_SERVER) $(TARGET_CLIENT) bin/

.PHONY: clean
clean:
	rm -f *.o $(TARGET)
