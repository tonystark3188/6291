MSGSERVER = dm_http 
MSGSERVER_FILES = standalone.c compat_unix.c
SRC_FUNC_PATH := ./http
MSGSERVER_FILES += $(wildcard $(SRC_FUNC_PATH)/*.c)

SRC_PPC_PATH := ./ppclib
MSGSERVER_FILES += $(wildcard $(SRC_PPC_PATH)/*.c)

FILE_TASK_PATH := ./file_task
MSGSERVER_FILES += $(wildcard $(FILE_TASK_PATH)/*.c)

TOOLS_PATH := ./tools
MSGSERVER_FILES += $(wildcard $(TOOLS_PATH)/*.c)

#HIDISKCLIENT = ppclib_sdk
#HIDISKCLIENT_FILES = test_main.c

#SRC_TOOLS_PATH = ./tools
#HIDISKCLIENT_FILES += $(wildcard $(SRC_TOOLS_PATH)/*.c)

#SRC_FILE_PATH = ./file_task
#HIDISKCLIENT_FILES += $(wildcard $(SRC_FILE_PATH)/*.c)

#SRC_PPCLIB_PATH = ./ppclib
#HIDISKCLIENT_FILES += $(wildcard $(SRC_PPCLIB_PATH)/*.c)


TARGET = $(MSGSERVER) $(HIDISKCLIENT)
all: $(TARGET)
$(MSGSERVER): $(MSGSERVER_FILES)
	$(CC) $(CFLAGS) $(LDFLAGS)  $(MSGSERVER_FILES) -ldl -ljson-c -lpthread  -lppc -o  $(MSGSERVER) -g

#$(HIDISKCLIENT): $(HIDISKCLIENT_FILES)
#	$(CC) $(CFLAGS) $(LDFLAGS)  $(HIDISKCLIENT_FILES) -ljson-c -o  $(HIDISKCLIENT) -g
	
.PHONY: clean
clean:
	rm -f *.o $(TARGET)
