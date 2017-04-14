

include $(TOPDIR)/rules.mk

PKG_NAME:=dm_crypto
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk


define Package/dm_crypto
  SECTION:=net
  CATEGORY:=Base system
  TITLE:=dm_crypto
  DEPENDS:=+libopenssl
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

TARGET_LDFLAGS += -L$(STAGING_DIR)/usr/lib -L$(STAGING_DIR)/lib -lcrypto #-L$(PKG_BUILD_DIR)/
TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include  

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
	CC="$(TARGET_CC)" \
	CFLAGS="$(TARGET_CFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)" \
	all
endef

define Package/dm_crypto/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/dm_crypto $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libdm_crypto.so $(1)/usr/lib
endef

$(eval $(call BuildPackage,dm_crypto))
