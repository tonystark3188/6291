MSGSERVER = dm_router
MSGSERVER_FILES = msg_server.c
SRC_FUNC_PATH := ./func
MSGSERVER_FILES += $(wildcard $(SRC_FUNC_PATH)/*.c)
SRC_SESSION_PATH := ./session
MSGSERVER_FILES += $(wildcard $(SRC_SESSION_PATH)/*.c)
SRC_ENCRYPT_PATH := ./encrypt
MSGSERVER_FILES += $(wildcard $(SRC_ENCRYPT_PATH)/*.c)
SRC_MD5_PATH := ./md5
MSGSERVER_FILES += $(wildcard $(SRC_MD5_PATH)/*.c)
SRC_PROCESS_PATH := ./api_process
MSGSERVER_FILES += $(wildcard $(SRC_PROCESS_PATH)/*.c)

SRC_SERVER_PATH := ./server
MSGSERVER_FILES += $(wildcard $(SRC_SERVER_PATH)/*.c)
SRC_TOOLS_PATH := ./tools
MSGSERVER_FILES += $(wildcard $(SRC_TOOLS_PATH)/*.c)
SRC_MSGLIB_PATH := ./msg_lib
MSGSERVER_FILES += $(wildcard $(SRC_MSGLIB_PATH)/*.c)

SRC_ROUTER_TASK_PATH := ./router_task
MSGSERVER_FILES += $(wildcard $(SRC_ROUTER_TASK_PATH)/*.c)
MSGSERVER_FILES += $(wildcard $(SRC_ROUTER_TASK_PATH)/func/*.c)
MSGSERVER_FILES += $(wildcard $(SRC_ROUTER_TASK_PATH)/func/disk/*.c)
MSGSERVER_FILES += $(wildcard $(SRC_ROUTER_TASK_PATH)/func/network/*.c)
MSGSERVER_FILES += $(wildcard $(SRC_ROUTER_TASK_PATH)/func/system/*.c)
MSGSERVER_FILES += $(wildcard $(SRC_ROUTER_TASK_PATH)/utils/*.c)

SRC_USR_MANAGE_PATH := ./usr_manage
MSGSERVER_FILES += $(wildcard $(SRC_USR_MANAGE_PATH)/*.c)

SRC_FILE_TASK_PATH := ./file_task
MSGSERVER_FILES += $(wildcard $(SRC_FILE_TASK_PATH)/*.c)
MSGSERVER_FILES += $(wildcard $(SRC_FILE_TASK_PATH)/server/*.c)
########SQLITE TASK START###################################
SRC_SQLITE_TASK_PATH := ./sqlite_task
MSGSERVER_FILES += $(wildcard $(SRC_SQLITE_TASK_PATH)/*.c)
MSGSERVER_FILES += $(wildcard $(SRC_SQLITE_TASK_PATH)/db/*.c)
MSGSERVER_FILES += $(wildcard $(SRC_SQLITE_TASK_PATH)/db_server_prcs/*.c)
MSGSERVER_FILES += $(wildcard $(SRC_SQLITE_TASK_PATH)/disk_change/*.c)
MSGSERVER_FILES += $(wildcard $(SRC_SQLITE_TASK_PATH)/disk_manage/*.c)
MSGSERVER_FILES += $(wildcard $(SRC_SQLITE_TASK_PATH)/business/*.c)
SRC_MQ_PATH := ./mq
MSGSERVER_FILES += $(wildcard $(SRC_MQ_PATH)/*.c)
SRC_MO_PATH := ./mo
MSGSERVER_FILES += $(wildcard $(SRC_MO_PATH)/*.c)
SRC_SM_PATH := ./sm
MSGSERVER_FILES += $(wildcard $(SRC_SM_PATH)/*.c)
SRC_TASK_PATH := ./task
MSGSERVER_FILES += $(wildcard $(SRC_TASK_PATH)/*.c)

SRC_EVENT_PATH := ./event
MSGSERVER_FILES += $(wildcard $(SRC_EVENT_PATH)/*.c)
SRC_CAMERA_PATH := ./camera
MSGSERVER_FILES += $(wildcard $(SRC_CAMERA_PATH)/*.c)


SRC_NETWORK_PATH := ./network
MSGSERVER_FILES += $(wildcard $(SRC_NETWORK_PATH)/*.c)
########SQLITE TASK END###################################




TARGET = $(MSGSERVER) 
all: $(TARGET)
$(MSGSERVER): $(MSGSERVER_FILES)
	$(CC) $(CFLAGS) $(LDFLAGS)  $(MSGSERVER_FILES) -Wall -lexif -lgphoto2 -lgphoto2_port -ldl -ljson-c -pthread -liw -luci -ludev -o  $(MSGSERVER)

.PHONY: clean
clean:
	rm -f *.o $(TARGET)
