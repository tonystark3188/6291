

include $(TOPDIR)/rules.mk

PKG_NAME:=dm_letv
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk


define Package/dm_letv
  SECTION:=net
  CATEGORY:=Base system
  TITLE:=dm_letv
  DEPENDS:=+libjson-c +libpthread +libuci
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
	$(CP) ./files/ $(PKG_BUILD_DIR)/
endef

TARGET_LDFLAGS += -L$(STAGING_DIR)/usr/lib 
TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include -I./include/ -I./include/tools/ -I./file_task/server -I./router_task -I./file_task

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
	CC="$(TARGET_CC)" \
	CFLAGS="$(TARGET_CFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)" \
	all
endef

define Package/dm_letv/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/dm_letv $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/dm_letv /media/srv32-home/home/fc/debug/
	$(INSTALL_BIN) ./files/$(PKG_NAME).init $(1)/etc/init.d/$(PKG_NAME)
	$(INSTALL_BIN) ./files/countFiles.sh $(1)/usr/sbin
	
endef

$(eval $(call BuildPackage,dm_letv))
