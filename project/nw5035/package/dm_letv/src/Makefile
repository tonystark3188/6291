MSGSERVER = dm_letv
MSGSERVER_FILES = main.c uci_for_cgi.c



SRC_TOOLS_PATH := ./tools
MSGSERVER_FILES += $(wildcard $(SRC_TOOLS_PATH)/*.c)
SRC_MD5_PATH := ./md5
MSGSERVER_FILES += $(wildcard $(SRC_MD5_PATH)/*.c)
SRC_MSGLIB_PATH := ./msg_lib
MSGSERVER_FILES += $(wildcard $(SRC_MSGLIB_PATH)/*.c)
SRC_ROUTER_TASK_PATH := ./router_task
MSGSERVER_FILES += $(wildcard $(SRC_ROUTER_TASK_PATH)/*.c)

SRC_FILE_TASK_PATH := ./file_task
MSGSERVER_FILES += $(wildcard $(SRC_FILE_TASK_PATH)/*.c)
MSGSERVER_FILES += $(wildcard $(SRC_FILE_TASK_PATH)/server/*.c)


SRC_HTTP_CLIENT_PATH := ./http_client
MSGSERVER_FILES += $(wildcard $(SRC_HTTP_CLIENT_PATH)/*.c)


TARGET = $(MSGSERVER) 
all: $(TARGET)
$(MSGSERVER): $(MSGSERVER_FILES)
	$(CC) $(CFLAGS) $(LDFLAGS)  $(MSGSERVER_FILES) -ldl -ljson-c -luci -pthread -o  $(MSGSERVER)

.PHONY: clean
clean:
	rm -f *.o $(TARGET)
