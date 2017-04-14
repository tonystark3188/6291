TARGET = erase_airdiskflag


all: $(TARGET) install

erase_airdiskflag: main.o 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ 
	
install:
	mkdir bin
	cp -f $(TARGET) bin/

 
.PHONY: clean
clean:
	rm -f *.o $(TARGET)
