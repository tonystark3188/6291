
include $(TOPDIR)/rules.mk

PKG_NAME:=libnassdk
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk


define Package/libnassdk
  SECTION:=net
  CATEGORY:=Base system
  TITLE:=lib NAS QQ SDK
  DEPENDS:=+libstdcpp +libpthread +zlib +libm +librt +libdl +libjson-c +glib2 
endef

define Package/libnassdk/description
  lib NAS QQ SDK
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)/lib
	mkdir -p $(PKG_BUILD_DIR)/etc/qq
	mkdir -p $(PKG_BUILD_DIR)/include
	$(CP) ./src/* $(PKG_BUILD_DIR)/
	$(CP) ./lib/* $(PKG_BUILD_DIR)/lib/
	$(CP) ./include/* $(PKG_BUILD_DIR)/include/
	$(CP) ./files/* $(PKG_BUILD_DIR)/etc/
endef
TARGET_CFLAGS += $(TARGET_CPPFLAGS) -I$(STAGING_DIR)/usr/include
TARGET_LDFLAGS += -L$(STAGING_DIR)/usr/lib

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
	CXX="$(TARGET_CXX)" \
	CFLAGS="$(TARGET_CFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)" \
	AR="$(TARGET_CROSS)ar " \
	all
endef

define Package/libnassdk/install
	$(INSTALL_DIR) $(1)/lib/
	$(INSTALL_DIR) $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/qq/
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libnas* $(1)/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lib/libtx* $(1)/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/nasdemo $(1)/usr/sbin/
	#$(INSTALL_BIN) $(PKG_BUILD_DIR)/smartlink/smartlink $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/nas_monitor/nasdemo_monitor $(1)/usr/sbin/
	$(CP) $(PKG_BUILD_DIR)/etc/*.pem $(1)/etc/qq/
	$(CP) $(PKG_BUILD_DIR)/etc/Guid_file.txt $(1)/etc/qq/
	$(CP) $(PKG_BUILD_DIR)/etc/licence.sign.file.txt $(1)/etc/qq/
	$(CP) $(PKG_BUILD_DIR)/etc/qq.init $(1)/etc/init.d/qq
	#$(CP) $(PKG_BUILD_DIR)/lib/lib* $(STAGING_DIR)/usr/lib
	#$(INSTALL_BIN) $(PKG_BUILD_DIR)/lib/libdmap.a $(1)/usr/lib/
	#$(INSTALL_BIN) $(PKG_BUILD_DIR)/demo/demo $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,libnassdk))
