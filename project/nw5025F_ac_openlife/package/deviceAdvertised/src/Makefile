DEVICE = deviceAdvertised
DEVICE_FILES = main.c upnp.c upnp_device.c upnp_renderer.c webserver.c xmlescape.c


TARGET = $(DEVICE)

DEVICE_OBJS = $(DEVICE_FILES:.c=.o)


all: $(TARGET) 

$(DEVICE): $(DEVICE_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

 
.PHONY: clean
clean:
	rm -f *.o $(TARGET)

