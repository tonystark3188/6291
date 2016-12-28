TARGETS := libnassdk.so
OBJS	:= $(patsubst %.cpp, %.o, $(wildcard *.cpp))

#如果您需要编译嵌入式平台下的libnassdk.so, 请设置CROSS变量
#CROSS   := arm-linux-gnueabihf-
CROSS   := 

#需要包含的头文件搜索路径
INCLUDE += -I./
INCLUDE += -I./include
INCLUDE += -I../../../include           # Tencent_iot_SDK/include
INCLUDE += -I./util
INCLUDE += -I./rapidjson
INCLUDE += -I./ft_processor


#需要引用的库
LIB     += -L./lib -ltxdevicesdk   # Tencent_iot_SDK/lib
LIB     += -lpthread  -lrt -ldl

export LD_LIBRARY_PATH='./lib"

BUILD_TYPE = -shared

#需要的标志位
#CXX	:= $(CROSS)g++
STRIP	:= $(TARGET_CROSS)strip
CXXFLAGS+= -fpermissive -Wall -Wno-deprecated -Wno-conversion-null -fpermissive -Wno-format  -std=c++0x  -fPIC
CXXFLAGS += -g
CXXFLAGS += $(BUILD_TYPE)

#增加根据d文件自动推导编译的标志
CXXFLAGS    += $(INCLUDE)


-include ft_processor/ft_processor.mk
-include util/util.mk

#deps
DEPS    += $(patsubst %.o, %.d, $(OBJS))

.PHONY: all
all: $(DEPS) $(OBJS)
	$(CXX) -o $(TARGETS) $(OBJS) $(LIB) $(BUILD_TYPE)
	$(CXX) NasSDKDemo.cpp  -o nasdemo -O0 -g3 -I"./include" $(CFLAGS) -L"./" -lnassdk -ljson-c -lpthread -lstdc++ -lm -lglib-2.0 -L"./lib" -ltxdevicesdk -std=c++0x
	#$(STRIP) $(TARGETS)
	$(CXX) nas_monitor/NasSDKDemo_monitor.cpp  -o nas_monitor/nasdemo_monitor

-include $(DEPS)

%.d: %.cpp
	@echo "make $@"
	@set -e $@ &&  rm -f $@; \
	$(CXX) -MM $(CFLAGS) $(CXXFLAGS) $< > $@; \

.PHONY:clean
clean:
	rm -rf $(TARGETS) $(OBJS) $(DEPS)

