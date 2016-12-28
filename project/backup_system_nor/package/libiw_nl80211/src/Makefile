MAKEFLAGS += --no-print-directory

PREFIX ?= /usr
SBINDIR ?= $(PREFIX)/sbin
MANDIR ?= $(PREFIX)/share/man
PKG_CONFIG ?= pkg-config

MKDIR ?= mkdir -p
INSTALL ?= install
CC ?= "gcc"
AR = ar

CFLAGS ?= -O2 -g
CFLAGS += -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration

TARGET_LIB = libiw_nl80211.so
TARGET_STATIC = libiw_nl80211.a
TARGET_TEST = scan_test
OBJS = iw.o genl.o event.o util.o scan.o reg.o status.o reason.o 
OBJS_TEST = scan_test.o
	
OBJS += sections.o

#ALL = $(TARGET_TEST) $(TARGET_LIB)
ALL = $(TARGET_TEST) $(TARGET_STATIC)

ifeq ($(NO_PKG_CONFIG),)
NL3xFOUND := $(shell $(PKG_CONFIG) --atleast-version=3.2 libnl-3.0 && echo Y)
ifneq ($(NL3xFOUND),Y)
NL31FOUND := $(shell $(PKG_CONFIG) --exact-version=3.1 libnl-3.1 && echo Y)
ifneq ($(NL31FOUND),Y)
NL3FOUND := $(shell $(PKG_CONFIG) --atleast-version=3 libnl-3.0 && echo Y)
ifneq ($(NL3FOUND),Y)
NL2FOUND := $(shell $(PKG_CONFIG) --atleast-version=2 libnl-2.0 && echo Y)
ifneq ($(NL2FOUND),Y)
NL1FOUND := $(shell $(PKG_CONFIG) --atleast-version=1 libnl-1 && echo Y)
endif
endif
endif
endif

ifeq ($(NL1FOUND),Y)
NLLIBNAME = libnl-1
endif

ifeq ($(NL2FOUND),Y)
CFLAGS += -DCONFIG_LIBNL20
LIBS += -lnl-genl
NLLIBNAME = libnl-2.0
endif

ifeq ($(NL3xFOUND),Y)
# libnl 3.2 might be found as 3.2 and 3.0
NL3FOUND = N
CFLAGS += -DCONFIG_LIBNL30
LIBS += -lnl-genl-3
NLLIBNAME = libnl-3.0
endif

ifeq ($(NL3FOUND),Y)
CFLAGS += -DCONFIG_LIBNL30
LIBS += -lnl-genl
NLLIBNAME = libnl-3.0
endif

# nl-3.1 has a broken libnl-gnl-3.1.pc file
# as show by pkg-config --debug --libs --cflags --exact-version=3.1 libnl-genl-3.1;echo $?
ifeq ($(NL31FOUND),Y)
CFLAGS += -DCONFIG_LIBNL30
LIBS += -lnl-genl
NLLIBNAME = libnl-3.1
endif

ifeq ($(NLLIBNAME),)
$(error Cannot find development files for any supported version of libnl)
endif

LIBS += $(shell $(PKG_CONFIG) --libs $(NLLIBNAME))
CFLAGS += $(shell $(PKG_CONFIG) --cflags $(NLLIBNAME))
endif # NO_PKG_CONFIG

all: $(ALL)

%.o: %.c iw.h nl80211.h
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET_TEST):	$(OBJS) $(OBJS_TEST)
	$(Q)$(CC) $(LDFLAGS) $(OBJS) $(OBJS_TEST) $(LIBS) -o $(TARGET_TEST)
	
$(TARGET_LIB):$(OBJS)
	$(Q)$(CC) $(LDFLAGS) $(OBJS) $(LIBS) -Wall -O2 -fPIC -shared -o $(TARGET_LIB)	

$(TARGET_STATIC):$(OBJS)
	$(AR) cru $@ $^

install: 
	mkdir bin
	mkdir lib
	cp -f $(TARGET_TEST) $(1)/bin/
	cp -f $(TARGET_STATIC) $(1)/lib/
clean:
	$(Q)rm -f $(TARGET_TEST) *.o *~
