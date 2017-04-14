#
# Copyright (C) 2007-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=newshair
PKG_VERSION:=0.1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk


define Package/newshair/Default
  SECTION:=net
  CATEGORY:=Base system
  TITLE:=broadcast for device!
endef

define Package/newshair/description
  newshair is a prj for broadcasting device.
endef

define Package/newshair
$(call Package/newshair/Default)
  DEPENDS:= +libpcre +libavahi-client +libopenssl +glib2 +libuci
endef

define Package/newshair/description
 .
 This package contains the Apache web server and utility programs.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

EXTRA_CFLAGS+= $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) -I$(STAGING_DIR)/usr/local/include
EXTRA_CFLAGS+= -I$(STAGING_DIR)/usr/include/glib-2.0 -DHAVE_CONFIG_H -I$(STAGING_DIR)/usr/include

GST_LIBS = -pthread -lm  -lglib-2.0 -lcrypto -lavahi-common -lavahi-client -luci

EXTRA_LDFLAGS+= $(TARGET_LDFLAGS) -Wl,-rpath-link,$(STAGING_DIR)/usr/lib $(GST_LIBS)


define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
	CC="$(TARGET_CC)" \
	CFLAGS="$(EXTRA_CFLAGS)" \
	LDFLAGS="$(EXTRA_LDFLAGS)" \
	all
endef



define Package/newshair/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/newshair $(1)/bin
	$(CP) ./files/check_shair.sh $(1)/usr/sbin
	$(CP) ./files/control_newshair.sh $(1)/usr/sbin
endef


$(eval $(call BuildPackage,newshair))
