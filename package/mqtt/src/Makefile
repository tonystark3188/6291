ifeq ($(OS),Windows_NT)
OSTYPE ?= $(OS)
MACHINETYPE ?= $(PROCESSOR_ARCHITECTURE)
echo OSTYPE is $(OSTYPE)
else
OSTYPE ?= $(shell uname -s)
MACHINETYPE ?= $(shell uname -m)
build.level = $(shell date)
endif # OS
ifeq ($(OSTYPE),linux)
OSTYPE = Linux
echo OSTYPE is $(OSTYPE)
endif

HIDISKCLIENT = mqtt
HIDISKCLIENT_FILES = test_main.c

SRC_CJSON_PATH = ./dmlibs/cjson/
HIDISKCLIENT_FILES += $(wildcard $(SRC_CJSON_PATH)/*.c)

SRC_LIB_HTTP_PATH = ./dmlibs/libghttp-1.0.9/
HIDISKCLIENT_FILES += $(wildcard $(SRC_LIB_HTTP_PATH)/*.c)

SRC_CLIENT_MQTT_PATH = ./dmlibs/dmmqtt/
HIDISKCLIENT_FILES += $(wildcard $(SRC_CLIENT_MQTT_PATH)/*.c)

SRC_DOWNLOAD_MQTT_PATH = ./dmlibs/dmdownload/
HIDISKCLIENT_FILES += $(wildcard $(SRC_DOWNLOAD_MQTT_PATH)/*.c)


SRC_LIB_MQTT_PATH = ./dmlibs/paho.mqtt.c-master/src/
SOURCE_FILES = $(wildcard $(SRC_LIB_MQTT_PATH)/*.c)






SOURCE_FILES_C = $(filter-out $(SRC_LIB_MQTT_PATH)/MQTTAsync.c $(SRC_LIB_MQTT_PATH)/MQTTVersion.c $(SRC_LIB_MQTT_PATH)/SSLSocket.c, $(SOURCE_FILES))
SOURCE_FILES_CS = $(filter-out $(SRC_LIB_MQTT_PATH)/MQTTAsync.c $(SRC_LIB_MQTT_PATH)/MQTTVersion.c, $(SOURCE_FILES))
SOURCE_FILES_A = $(filter-out $(SRC_LIB_MQTT_PATH)/MQTTClient.c $(SRC_LIB_MQTT_PATH)/MQTTVersion.c $(SRC_LIB_MQTT_PATH)/SSLSocket.c, $(SOURCE_FILES))
SOURCE_FILES_AS = $(filter-out $(SRC_LIB_MQTT_PATH)/MQTTClient.c $(SRC_LIB_MQTT_PATH)/MQTTVersion.c, $(SOURCE_FILES))
HIDISKCLIENT_FILES += $(SOURCE_FILES_A)

#HIDISKCLIENT_FILES += $(SRC_LIB_MQTT_PATH)/samples/MQTTClient_subscribe.c



ifeq ($(OSTYPE),Linux)

MQTTCLIENT_INIT = MQTTClient_init
MQTTASYNC_INIT = MQTTAsync_init


EXTRA_LIB = -ldl

else ifeq ($(OSTYPE),Darwin)

MQTTCLIENT_INIT = MQTTClient_init
MQTTASYNC_INIT = MQTTAsync_init

EXTRA_LIB = -ldl

endif



TARGET = $(HIDISKCLIENT)
CC = gcc
CFLAGS = -I./  -I/usr/local/include/  -I./dmlibs/cjson/ -I./dmlibs/include/ -I./dmlibs/libghttp-1.0.9/ -I./dmlibs/paho.mqtt.c-master/src/ -I./dmlibs/dmdownload/ -I./dmlibs/dmmqtt/
CFLAGS += -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE
LDFLAGS = -L/usr/local/lib/ -Wl, -init,$(MQTTCLIENT_INIT)
all: $(TARGET)
$(HIDISKCLIENT): $(HIDISKCLIENT_FILES)
	$(CC) $(CFLAGS) $(LDFLAGS)  $(HIDISKCLIENT_FILES) -pthread -ljson-c -lm -lssl -lcrypto -o  $(HIDISKCLIENT)
.PHONY: clean

clean:
	rm -f *.o $(TARGET)
