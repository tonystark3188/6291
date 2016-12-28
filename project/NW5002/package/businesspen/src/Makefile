PENSERVER = penServer
DMICRECORD = dmicRecord
MYPCM2MP3 = mypcm2mp3
FORK = myfork

FORK_FILES = fork.c
PENSERVER_FILES = main_tcp.c mysignal.c tcp_server.c tcp_server_send.c
DMICRECORD_FILES = ./record/main_record.c ./record/pcm2mp3.c ./record/filemanager.c ./record/record_list.c ./record/wav.c ./mysignal.c ./record/audio_format_convert.c ./record/amrnb.c
MYPCM2MP3_FILES = mypcm2mp3.c

TARGET = $(DMICRECORD) $(PENSERVER) $(FORK)#$(MYPCM2MP3) 

FORK_OBJS = $(FORK_FILES:.c=.o)
PENSERVER_OBJS = $(PENSERVER_FILES:.c=.o)
DMICRECORD_OBJS = $(DMICRECORD_FILES:.c=.o)
MYPCM2MP3_OBJS = $(MYPCM2MP3_FILES:.c=.o)

all: $(TARGET) 

$(DMICRECORD): $(DMICRECORD_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -I ./record -L$(STAGING_DIR)/usr/lib  -L$(STAGING_DIR)/usr/local/lib -lmp3lame -lspeex -lspeexdsp
$(MYPCM2MP3): $(MYPCM2MP3_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -lmp3lame
$(PENSERVER): $(PENSERVER_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
$(FORK): $(FORK_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
	
.PHONY: clean
clean:
	rm -f *.o $(TARGET)

