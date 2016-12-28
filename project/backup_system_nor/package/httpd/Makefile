

include $(TOPDIR)/rules.mk

PKG_NAME:=httpd
PKG_VERSION:=2
PKG_RELEASE:=2

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk


define Package/httpd
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Web Servers/Proxies
  TITLE:=apache httpd- HTTP server configuration
  MAINTAINER:=Jo-Philipp Wich <xm@subsignal.org>
  DEPENDS:=+libpthread +librt +libopenssl +libuuid
endef

define Package/httpd/description
 Apache HTTPd.It is intended as a drop-in replacement for the Busybox
 HTTP daemon.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./mips/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	echo "Compile httpd conf package..."
endef

define Package/httpd/install
	echo "install httpd package..."
	#$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/usr/mips
	$(INSTALL_DIR) $(1)/usr/mips/cgi-bin
	$(INSTALL_DIR) $(1)/usr/mips/conf
	$(INSTALL_DIR) $(1)/usr/mips/icons
	$(INSTALL_DIR) $(1)/usr/mips/www
	#$(INSTALL_DIR) $(1)/bin
	#$(INSTALL_DIR) $(1)/lib
	#$(CP) $(PKG_BUILD_DIR)/cgi-bin/* $(1)/usr/mips/cgi-bin/
	$(CP) $(PKG_BUILD_DIR)/conf/* $(1)/usr/mips/conf/
	$(CP) $(PKG_BUILD_DIR)/icons/* $(1)/usr/mips/icons/
	$(CP) $(PKG_BUILD_DIR)/www_for_airdisk/* $(1)/usr/mips/www/
	$(CP) $(PKG_BUILD_DIR)/user.passwd $(1)/usr/mips/
	$(CP) $(PKG_BUILD_DIR)/script/* $(1)/etc/init.d/
	#$(CP) $(PKG_BUILD_DIR)/lib/* $(1)/lib
	#$(CP) $(PKG_BUILD_DIR)/bin/* $(1)/bin
	
endef

$(eval $(call BuildPackage,httpd))
