


include $(TOPDIR)/rules.mk

PKG_NAME:=dm_router
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk


define Package/dm_router
  SECTION:=net
  CATEGORY:=Base system
  TITLE:=dm_router
  DEPENDS:=+libdl +libpthread +libjson-c +libuci +udev +libwifi_uart +libiw +libm +libnl-tiny +libiw_nl80211
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
	$(CP) ./files/ $(PKG_BUILD_DIR)/
endef

TARGET_LDFLAGS += -L$(STAGING_DIR)/usr/lib -L$(STAGING_DIR)/lib -L$(BUILD_DIR_TOOLCHAIN)/uClibc-0.9.33.2/lib
TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include -I./include/ -I./include/tools/\
-I./router_task/ -I./usr_manage/ -I./process_json/ -I./api_process/ -I./server/ -I./session_manage -I./pwd_manage -I./

TARGET_CFLAGS += -I./router_task/func/ -I./router_task/func/disk/ -I./router_task/func/network/\
-I./router_task/func/system/ -I./router_task/utils/ -I./router_task/func/mcu/ -I./router_task/func/nor/

TARGET_CFLAGS += -I./file_task/ -I./file_task/server/ -I./sqlite_task/db/include/ -I./sqlite_task/business/\
-I./sqlite_task/db_server_prcs/ -I./sqlite_task/disk_manage/ -I./sqlite_task/disk_change/ -I./task/ -I./fatattr/

TARGET_CFLAGS += -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -Wl,--gc-sections

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
	CC="$(TARGET_CC)" \
	CFLAGS="$(TARGET_CFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)" \
	all
endef

define Package/dm_router/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/usr/mips/conf/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/dm_router $(1)/usr/sbin
	$(INSTALL_BIN) ./files/$(PKG_NAME).init $(1)/etc/init.d/$(PKG_NAME)
	$(INSTALL_BIN) ./files/$(PKG_NAME).conf $(1)/usr/mips/conf/
endef

$(eval $(call BuildPackage,dm_router))
