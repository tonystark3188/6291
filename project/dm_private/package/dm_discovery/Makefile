

include $(TOPDIR)/rules.mk

PKG_NAME:=dm_discovery
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk


define Package/dm_discovery
  SECTION:=net
  CATEGORY:=Base system
  TITLE:=dm_discovery
  DEPENDS:=+libpthread +libjson-c
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
	$(CP) ./files/ $(PKG_BUILD_DIR)/
endef

TARGET_LDFLAGS += -L$(STAGING_DIR)/usr/lib -L$(STAGING_DIR)/lib 
TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include  -I./process_json/ -I./usr_manage/ -I./tools/ -I./msg_lib/ -I./server/\
-I./api_process/ -I./include/ 


define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
	CC="$(TARGET_CC)" \
	CFLAGS="$(TARGET_CFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)" \
	all
endef

define Package/dm_discovery/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/dm_discovery $(1)/usr/sbin
	$(INSTALL_BIN) ./files/$(PKG_NAME).init $(1)/etc/init.d/$(PKG_NAME)
endef

$(eval $(call BuildPackage,dm_discovery))
