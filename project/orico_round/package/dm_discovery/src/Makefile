MSGSERVER = dm_discovery
MSGSERVER_FILES = msg_server.c

SRC_PROCESS_PATH := ./api_process
MSGSERVER_FILES += $(wildcard $(SRC_PROCESS_PATH)/*.c)

MSGSERVER_FILES += $(wildcard $(SRC_SQLITE_OPR_PATH)/*.c)
SRC_PROCESS_JSON_PATH := ./process_json
MSGSERVER_FILES += $(wildcard $(SRC_PROCESS_JSON_PATH)/*.c)
SRC_SERVER_PATH := ./server
MSGSERVER_FILES += $(wildcard $(SRC_SERVER_PATH)/*.c)
SRC_TOOLS_PATH := ./tools
MSGSERVER_FILES += $(wildcard $(SRC_TOOLS_PATH)/*.c)
SRC_MSGLIB_PATH := ./msg_lib
MSGSERVER_FILES += $(wildcard $(SRC_MSGLIB_PATH)/*.c)

SRC_USR_PATH := ./usr_manage
MSGSERVER_FILES += $(wildcard $(SRC_USR_PATH)/*.c)


TARGET = $(MSGSERVER) 
all: $(TARGET)
$(MSGSERVER): $(MSGSERVER_FILES)
	$(CC) $(CFLAGS) $(LDFLAGS)  $(MSGSERVER_FILES)  -ljson-c -pthread -o  $(MSGSERVER)

.PHONY: clean
clean:
	rm -f *.o $(TARGET)
