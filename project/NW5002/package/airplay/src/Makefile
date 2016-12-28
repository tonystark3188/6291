AIRPLAY = airplay
CFLAGS += -I. -fPIC -g -Wall
SRCS := main.c shairport.c daemon.c rtsp.c mdns.c mdns_external.c common.c rtp.c metadata.c \
              player.c alac.c audio.c audio_dummy.c audio_pipe.c
			

CONFIG_OSS=yes
CONFIG_FANCY_RESAMPLE=no
CONFIG_AVAHI=no
CONFIG_HAVE_DNS_SD_H=no
CONFIG_FASTMDNS=yes
CONFIG_TINYSVCMDNS=yes
CONFIG_HAVE_GETOPT_H=yes

ifeq ($(CONFIG_FASTMDNS), yes)
SRCS += mdns_fastmdns.c 
SRCS += fastmdns.c
SRCS += mcast.c
endif


ifeq ($(CONFIG_TINYSVCMDNS), yes)
SRCS += mdns_tinysvcmdns.c
SRCS += tinysvcmdns.c
endif

ifeq ($(CONFIG_OSS), yes)
SRCS += audio_oss.c
endif

TARGET = $(AIRPLAY)

AIRPLAY_OBJS = $(SRCS:.c=.o)


all: $(TARGET) 

$(AIRPLAY): $(AIRPLAY_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

 
.PHONY: clean
clean:
	rm -f *.o $(TARGET)
