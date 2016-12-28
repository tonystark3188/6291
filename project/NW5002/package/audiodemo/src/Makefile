AUDIODEMO = audiodemo
AUDIODEMO_FILES = audiodemo.c wav.c
TARGET = $(AUDIODEMO)

AUDIODEMO_OBJS = $(AUDIODEMO_FILES:.c=.o)

all: $(TARGET) 

$(AUDIODEMO): $(AUDIODEMO_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
	
.PHONY: clean
clean:
	rm -f *.o $(TARGET)

