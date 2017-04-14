NEWSHARE = newshair

SRCS := shairport.c daemon.c rtsp.c mdns.c mdns_external.c mdns_tinysvcmdns.c common.c rtp.c player.c tinysvcmdns.c \
uci_for_render.c metadataconfig.c
			

SRCS += mdns_avahi.c

SRCS += metadata.c

TARGET = $(NEWSHARE)

NEWSHARE_OBJS = $(SRCS:.c=.o)


all: $(TARGET) 

$(NEWSHARE): $(NEWSHARE_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

 
.PHONY: clean
clean:
	rm -f *.o $(TARGET)
