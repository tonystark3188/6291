MSGSERVER = dm_http
MSGSERVER_FILES = standalone.c compat_unix.c
SRC_FUNC_PATH := ./http
MSGSERVER_FILES += $(wildcard $(SRC_FUNC_PATH)/*.c)

TARGET = $(MSGSERVER) 
all: $(TARGET)
$(MSGSERVER): $(MSGSERVER_FILES)
	$(CC) $(CFLAGS) $(LDFLAGS)  $(MSGSERVER_FILES) -ldl -o  $(MSGSERVER)

.PHONY: clean
clean:
	rm -f *.o $(TARGET)