include $(TOPDIR)/rules.mk

PKG_NAME:=businesspen
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk


define Package/businesspen
  SECTION:=net
  CATEGORY:=Base system
  TITLE:=businesspen program
  DEPENDS:= +libpthread +libmp3lame +opencore-amr +libspeex +libspeexdsp
endef

define Package/businesspen/description
  businesspen program
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include -I$(KERNEL_BUILD_DIR) -I$(KERNEL_BUILD_DIR)/include -I ./
TARGET_LDFLAGS += -L$(STAGING_DIR)/usr/lib  -lm -lpthread -lopencore-amrnb -lopencore-amrwb

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
	CC="$(TARGET_CC)" \
	CFLAGS="$(TARGET_CFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)" \
	all
endef

define Package/businesspen/install
	$(INSTALL_DIR) $(1)/bin
#	$(INSTALL_BIN) $(PKG_BUILD_DIR)/penServer $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/dmicRecord $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/penServer $(1)/bin
#	$(INSTALL_BIN) $(PKG_BUILD_DIR)/myfork $(1)/bin
#	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mypcm2mp3 $(1)/bin
	
endef

$(eval $(call BuildPackage,businesspen))
