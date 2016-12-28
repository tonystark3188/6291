


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
  DEPENDS:=+libdl +libpthread +libjson-c +libiw +libuci +udev +libexif +libgphoto2 +libgphoto2_port
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
	$(CP) ./files/ $(PKG_BUILD_DIR)/
endef

TARGET_LDFLAGS += -L$(STAGING_DIR)/usr/lib -L$(STAGING_DIR)/lib 
TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include -I./include/ -I./include/tools/\
-I./router_task/ -I./usr_manage/ -I./camera -I./

TARGET_CFLAGS += -I./router_task/func/ -I./router_task/func/disk/ -I./router_task/func/network/\
-I./router_task/func/system/ -I./router_task/utils/

TARGET_CFLAGS += -I./file_task/ -I./file_task/server/ -I./sqlite_task/db/include/ -I./sqlite_task/business/\
-I./sqlite_task/db_server_prcs/ -I./sqlite_task/disk_manage/ -I./sqlite_task/disk_change/



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
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/dm_router $(1)/usr/sbin
	$(INSTALL_BIN) ./files/$(PKG_NAME).init $(1)/etc/init.d/$(PKG_NAME)
endef

$(eval $(call BuildPackage,dm_router))
