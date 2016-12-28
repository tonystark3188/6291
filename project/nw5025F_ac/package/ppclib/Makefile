

include $(TOPDIR)/rules.mk

PKG_NAME:=ppclib
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk


define Package/ppclib
  SECTION:=net
  CATEGORY:=Base system
  TITLE:=ppclib
  DEPENDS:=+libpthread +libjson-c +librt
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
	$(CP) ./files/ $(PKG_BUILD_DIR)/
endef

TARGET_LDFLAGS += -L$(STAGING_DIR)/usr/lib 
TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include -I./include/ -I./ppclib/ -I./ -I./tools/ -I./file_task/ 

TARGET_CFLAGS += -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -Wl,--gc-sections

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
	CC="$(TARGET_CC)" \
	CFLAGS="$(TARGET_CFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)" \
	all
endef

$(warning linzejia $(STAGING_DIR) ; $(PKG_BUILD_DIR))
define Package/ppclib/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include/$(PKG_NAME)
	#$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libppc_test $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/*.so $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/*.a $(STAGING_DIR)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/ppclib/*.h $(STAGING_DIR)/usr/include/$(PKG_NAME)
	$(CP) $(PKG_BUILD_DIR)/tools/*.h $(STAGING_DIR)/usr/include/$(PKG_NAME)
	#$(INSTALL_BIN) ./files/$(PKG_NAME).init $(1)/etc/init.d/$(PKG_NAME)
	
endef

$(eval $(call BuildPackage,ppclib))
