#
# Copyright (C) 2007-2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# blogic@openwrt.org

include $(TOPDIR)/rules.mk

PKG_NAME:=libmp3lame
PKG_VERSION:=3.99.5
PKG_RELEASE:=2
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/lame
PKG_MD5SUM:=04FEEBD857A591E009DFD565C6FBEC17

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk

PKG_INSTALL=1

define Package/libmp3lame
  SECTION:=multimedia
  CATEGORY:=Multimedia
  URL:=http://sourceforge.net/projects/lame
  TITLE:=libmp3lame
endef

define Package/libmp3lame/description
lame mp3 encoder
endef

define Build/Configure
	$(call Build/Configure/Default, \
	--disable-decoder \
	)
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		all
endef

define Package/libmp3lame/install
#	$(INSTALL_DIR) $(1)/usr/bin
#	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/lame $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/*.{la,so*} \
		$(1)/usr/lib/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/lame
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/include/lame/*.h \
		$(1)/usr/include/lame/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/*.{la,so*} \
		$(1)/usr/lib/
endef

$(eval $(call BuildPackage,libmp3lame))
