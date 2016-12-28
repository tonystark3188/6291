

include $(TOPDIR)/rules.mk

PKG_NAME:=audiodemo
PKG_VERSION:=1.0
PKG_RELEASE:=1

#export ENCRYPT_EN=OK
export ENCRYPT_EN=

ifdef ENCRYPT_EN
TARGET_CFLAGS+= -D ENCRYPT_ENABLE
endif

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk


define Package/audiodemo
  SECTION:=net
  CATEGORY:=Base system
  TITLE:=audiodemo program for audio develop
  DEPENDS:= +libuci +libiw +udev
  
endef

define Package/audiodemo/description
  audiodemo program for web interface
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include -I$(KERNEL_BUILD_DIR) -I$(KERNEL_BUILD_DIR)/include
TARGET_LDFLAGS += -L$(STAGING_DIR)/usr/lib  -lm -lpthread

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
	CC="$(TARGET_CC)" \
	CFLAGS="$(TARGET_CFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)" \
	all
endef

define Package/audiodemo/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/audiodemo $(1)/bin

	
endef

$(eval $(call BuildPackage,audiodemo))
