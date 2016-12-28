MSGSERVER = dm_nas
MSGSERVER_FILES = msg_server.c

SRC_TOKEN_PATH := ./token
MSGSERVER_FILES += $(wildcard $(SRC_TOKEN_PATH)/*.c)
SRC_ENCRYPT_PATH := ./encrypt
MSGSERVER_FILES += $(wildcard $(SRC_ENCRYPT_PATH)/*.c)
SRC_MD5_PATH := ./md5
MSGSERVER_FILES += $(wildcard $(SRC_MD5_PATH)/*.c)
SRC_PROCESS_PATH := ./api_process
MSGSERVER_FILES += $(wildcard $(SRC_PROCESS_PATH)/*.c)

SRC_PROCESS_JSON_PATH := ./process_json
MSGSERVER_FILES += $(wildcard $(SRC_PROCESS_JSON_PATH)/*.c)
SRC_SERVER_PATH := ./server
MSGSERVER_FILES += $(wildcard $(SRC_SERVER_PATH)/*.c)

SRC_MSGLIB_PATH := ./msg_lib
MSGSERVER_FILES += $(wildcard $(SRC_MSGLIB_PATH)/*.c)

SRC_ROUTER_TASK_PATH := ./router_task
MSGSERVER_FILES += $(wildcard $(SRC_ROUTER_TASK_PATH)/*.c)
MSGSERVER_FILES += $(wildcard $(SRC_ROUTER_TASK_PATH)/func/*.c)
MSGSERVER_FILES += $(wildcard $(SRC_ROUTER_TASK_PATH)/func/disk/*.c)
MSGSERVER_FILES += $(wildcard $(SRC_ROUTER_TASK_PATH)/func/network/*.c)
MSGSERVER_FILES += $(wildcard $(SRC_ROUTER_TASK_PATH)/func/system/*.c)
MSGSERVER_FILES += $(wildcard $(SRC_ROUTER_TASK_PATH)/func/mcu/*.c)
MSGSERVER_FILES += $(wildcard $(SRC_ROUTER_TASK_PATH)/func/nor/*.c)
MSGSERVER_FILES += $(wildcard $(SRC_ROUTER_TASK_PATH)/utils/*.c)

SRC_USR_MANAGE_PATH := ./usr_manage
MSGSERVER_FILES += $(wildcard $(SRC_USR_MANAGE_PATH)/*.c)

SRC_TOKEN_MANAGE_PATH := ./token_manage
MSGSERVER_FILES += $(wildcard $(SRC_TOKEN_MANAGE_PATH)/*.c)

SRC_MEDIA_MANAGE_PATH := ./media_manage
MSGSERVER_FILES += $(wildcard $(SRC_MEDIA_MANAGE_PATH)/*.c)


########FILE TASK START###################################
SRC_FILE_TASK_PATH := ./file_task
MSGSERVER_FILES += $(wildcard $(SRC_FILE_TASK_PATH)/*.c)
MSGSERVER_FILES += $(wildcard $(SRC_FILE_TASK_PATH)/server/*.c)
########SQLITE TASK START###################################
SRC_CLOUD_TOOLS_PATH := ./cloud_tools
MSGSERVER_FILES += $(wildcard $(SRC_CLOUD_TOOLS_PATH)/*.c)
SRC_CLOUD_TASK_PATH := ./cloud_db
MSGSERVER_FILES += $(wildcard $(SRC_CLOUD_TASK_PATH)/*.c)
MSGSERVER_FILES += $(wildcard $(SRC_CLOUD_TASK_PATH)/business/*.c)
MSGSERVER_FILES += $(wildcard $(SRC_CLOUD_TASK_PATH)/mo/*.c)
MSGSERVER_FILES += $(wildcard $(SRC_CLOUD_TASK_PATH)/mq/*.c)
MSGSERVER_FILES += $(wildcard $(SRC_CLOUD_TASK_PATH)/sm/*.c)
MSGSERVER_FILES += $(wildcard $(SRC_CLOUD_TASK_PATH)/task/*.c)
MSGSERVER_FILES += $(wildcard $(SRC_CLOUD_TASK_PATH)/db_prcs/*.c)

SRC_CLOUD_SQLITE_PATH := ./cloud_db/sqlite_db
MSGSERVER_FILES += $(wildcard $(SRC_CLOUD_SQLITE_PATH)/*.c)
MSGSERVER_FILES += $(wildcard $(SRC_CLOUD_SQLITE_PATH)/authority_table/*.c)
MSGSERVER_FILES += $(wildcard $(SRC_CLOUD_SQLITE_PATH)/bucket_table/*.c)
MSGSERVER_FILES += $(wildcard $(SRC_CLOUD_SQLITE_PATH)/db/*.c)
MSGSERVER_FILES += $(wildcard $(SRC_CLOUD_SQLITE_PATH)/disk_table/*.c)
MSGSERVER_FILES += $(wildcard $(SRC_CLOUD_SQLITE_PATH)/file_table/*.c)
MSGSERVER_FILES += $(wildcard $(SRC_CLOUD_SQLITE_PATH)/media_table/*.c)
MSGSERVER_FILES += $(wildcard $(SRC_CLOUD_SQLITE_PATH)/user_table/*.c)
MSGSERVER_FILES += $(wildcard $(SRC_CLOUD_SQLITE_PATH)/v_file_table/*.c)
MSGSERVER_FILES += $(wildcard $(SRC_CLOUD_SQLITE_PATH)/version_table/*.c)
MSGSERVER_FILES += $(wildcard $(SRC_CLOUD_SQLITE_PATH)/db_update/*.c)
MSGSERVER_FILES += $(wildcard $(SRC_CLOUD_SQLITE_PATH)/db_backup/*.c)

SRC_CLOUD_RFSVFS_PATH := ./rfsvfs
MSGSERVER_FILES += $(wildcard $(SRC_CLOUD_RFSVFS_PATH)/*.c)

SRC_CLOUD_BFAVFS_PATH := ./bfavfs
MSGSERVER_FILES += $(wildcard $(SRC_CLOUD_BFAVFS_PATH)/*.c)

SRC_CLOUD_AFVFS_PATH := ./afvfs
MSGSERVER_FILES += $(wildcard $(SRC_CLOUD_AFVFS_PATH)/*.c)

SRC_CLOUD_MEDIA_PRC_PATH := ./media_process
MSGSERVER_FILES += $(wildcard $(SRC_CLOUD_MEDIA_PRC_PATH)/*.c)
MSGSERVER_FILES += $(wildcard $(SRC_CLOUD_MEDIA_PRC_PATH)/thumbnail/*.c)
MSGSERVER_FILES += $(wildcard $(SRC_CLOUD_MEDIA_PRC_PATH)/media_info/*.c)
MSGSERVER_FILES += $(wildcard $(SRC_CLOUD_MEDIA_PRC_PATH)/media_snapshot/*.c)

Local_LDFLAGS := -ljpeg -lpng -lgif -lexif -lavformat -lavcodec -lavutil -lswscale
Local_CFLAGS := -I$(SRC_CLOUD_MEDIA_PRC_PATH) -I$(SRC_CLOUD_MEDIA_PRC_PATH)/thumbnail -I$(SRC_CLOUD_MEDIA_PRC_PATH)/media_info
Local_CFLAGS += -I$(SRC_CLOUD_MEDIA_PRC_PATH)/media_snapshot

SRC_PINYIN_PATH := ./pinyin
MSGSERVER_FILES += $(wildcard $(SRC_PINYIN_PATH)/*.c)



TARGET = $(MSGSERVER) 
all: $(TARGET)
$(MSGSERVER): $(MSGSERVER_FILES)
	$(CC) $(CFLAGS) $(Local_CFLAGS) $(Local_LDFLAGS) $(LDFLAGS)  $(MSGSERVER_FILES) -ldl -ljson-c -pthread -luci -ludev -lwifi_uart -liw -liw_nl80211 -lm -lnl-tiny -o  $(MSGSERVER) 

.PHONY: clean
clean:
	rm -f *.o $(TARGET)
