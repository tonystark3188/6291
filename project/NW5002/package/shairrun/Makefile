

include $(TOPDIR)/rules.mk

PKG_NAME:=shairrun
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk


define Package/shairrun
  SECTION:=sound
  CATEGORY:=Sound
  TITLE:=shairrun  interface
endef

define Package/shairrun/description
  shairrun interface
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
endef

define Package/shairrun/install
	$(INSTALL_DIR) $(1)/usr
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/config
	$(CP) $(PKG_BUILD_DIR)/shair $(1)/etc/config/shair
	$(CP) $(PKG_BUILD_DIR)/checkserverPen $(1)/usr/bin/checkserverPen
	$(CP) $(PKG_BUILD_DIR)/penServerrestart $(1)/usr/bin/penServerrestart
	$(CP) $(PKG_BUILD_DIR)/recordrestart $(1)/usr/bin/recordrestart
endef

$(eval $(call BuildPackage,shairrun))
