TARGET = check_reset


all: $(TARGET) install

check_reset: main.o 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -lwifi_uart
	
install:
	mkdir bin
	cp -f $(TARGET) bin/

 
.PHONY: clean
clean:
	rm -f *.o $(TARGET)
