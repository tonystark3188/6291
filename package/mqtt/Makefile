include $(TOPDIR)/rules.mk

PKG_NAME:=mqtt
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk


define Package/mqtt
  SECTION:=net
  CATEGORY:=Base system
  TITLE:=mqtt
  DEPENDS:=+libpthread +libjson-c +libopenssl
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
	#$(CP) ./files/ $(PKG_BUILD_DIR)/
endef

TARGET_LDFLAGS += -L$(STAGING_DIR)/usr/lib 
TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include  -I./ -I./dmlibs/include -I./dmlibs/dmmqtt -I./dmlibs/dmdownload -I./dmlibs/dmuser -I./dmlibs/dmota -I./dmlibs/libghttp-1.0.9 -I./dmlibs/paho.mqtt.c-master/src
TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include/json-c 

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
	CC="$(TARGET_CC)" \
	CFLAGS="$(TARGET_CFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)" \
	all
endef

define Package/mqtt/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/usr/mips/conf/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mqtt $(1)/usr/sbin
	$(INSTALL_BIN) ./files/$(PKG_NAME).init $(1)/etc/init.d/$(PKG_NAME)

endef

$(eval $(call BuildPackage,mqtt))
