include $(TOPDIR)/rules.mk

PKG_NAME:=deviceAdvertised
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk


define Package/deviceAdvertised
  SECTION:=net
  CATEGORY:=Base system
  TITLE:=deviceAdvertised for airdisk3.0
  DEPENDS:= +glib2 +libupnp +libixml
endef

define Package/deviceAdvertised/description
  deviceAdvertised for airdisk3.0
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
	mkdir -p $(PKG_BUILD_DIR)/data
	$(CP) ./data/* $(PKG_BUILD_DIR)/data
endef

EXTRA_CFLAGS+= $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) -I$(STAGING_DIR)/usr/local/include
EXTRA_CFLAGS+= -I$(STAGING_DIR)/usr/include/glib-2.0 -DHAVE_CONFIG_H 


GST_LIBS = -pthread -lm -lupnp -lglib-2.0 -lixml

EXTRA_LDFLAGS+= $(TARGET_LDFLAGS) -Wl,-rpath-link,$(STAGING_DIR)/usr/lib $(GST_LIBS)


define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
	CC="$(TARGET_CC)" \
	CFLAGS="$(EXTRA_CFLAGS)" \
	LDFLAGS="$(EXTRA_LDFLAGS)" \
	all
endef

define Package/deviceAdvertised/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_DIR) $(1)/usr
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/share
	$(INSTALL_DIR) $(1)/usr/share/airdisk
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DIR) $(1)/etc/init.d
	$(CP) $(PKG_BUILD_DIR)/deviceAdvertised $(1)/bin
	$(CP) $(PKG_BUILD_DIR)/adverrestart $(1)/usr/bin/adverrestart
	$(CP) $(PKG_BUILD_DIR)/data/grender-64x64.png $(1)/usr/share/airdisk
	$(CP) $(PKG_BUILD_DIR)/data/grender-128x128.png $(1)/usr/share/airdisk
endef

$(eval $(call BuildPackage,deviceAdvertised))
