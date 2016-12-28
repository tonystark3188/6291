TARGET = check_reset


all: $(TARGET) install

check_reset: main.o uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -lwifi_uart -luci
	
install:
	mkdir bin
	cp -f $(TARGET) bin/

 
.PHONY: clean
clean:
	rm -f *.o $(TARGET)
