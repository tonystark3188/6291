TARGET = check_reset


all: $(TARGET) install

check_reset: main.o uci_for_cgi.o net_speed.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci -lnotify_server 
	#-lwifi_uart
	
install:
	mkdir bin
	cp -f $(TARGET) bin/

 
.PHONY: clean
clean:
	rm -f *.o $(TARGET)
