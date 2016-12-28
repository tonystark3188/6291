TARGET = $(TARGET_SERVER) $(TARGET_CLIENT) $(TARGET_LIB)
TARGET_SERVER = uart_server
TARGET_CLIENT = uart_client_test
TARGET_LIB = libwifi_uart.so
OBJS_LIB = socket_uart.c wifi_uart.c msg.c
OBJS_SERVER = uart_server.c
OBJS_CLIENT = uart_client_test.c

all: $(TARGET) install

$(TARGET_LIB):$(OBJS_LIB)
	$(CC) -Wall -O2 -fPIC -shared  -o $@ $^ 
	
$(TARGET_SERVER): $(OBJS_LIB) $(OBJS_SERVER)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ 
	
$(TARGET_CLIENT): $(OBJS_LIB) $(OBJS_CLIENT)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ 


install:
	mkdir lib
	mkdir bin
	cp -f $(TARGET_LIB) lib/
	cp -f $(TARGET_SERVER) $(TARGET_CLIENT) bin/

 
.PHONY: clean
clean:
	rm -f *.o $(TARGET)
