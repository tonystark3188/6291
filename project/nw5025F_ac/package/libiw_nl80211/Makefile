include $(TOPDIR)/rules.mk

PKG_NAME:=libiw_nl80211
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

#TARGET_CFLAGS += -DCONFIG_LIBNL20
#NLLIBNAME = libnl-2.0

#TARGET_CFLAGS += -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration
#TARGET_CFLAGS += -L/home/lzj/x1000_hcy/x1000_2/x1000/staging_dir/target-mipsel_mips32_uClibc-0.9.33.2/usr/lib -L/home/lzj/x1000_hcy/x1000_2/x1000/staging_dir/target-mipsel_mips32_uClibc-0.9.33.2/lib -L/home/lzj/x1000_hcy/x1000_2/x1000/staging_dir/toolchain-mipsel_mips32_gcc-4.6-linaro_uClibc-0.9.33.2/usr/lib -L/home/lzj/x1000_hcy/x1000_2/x1000/staging_dir/toolchain-mipsel_mips32_gcc-4.6-linaro_uClibc-0.9.33.2/lib -Wl,--gc-sections
#TARGET_CFLAGS += -I/home/lzj/x1000_hcy/x1000_2/x1000/staging_dir/target-mipsel_mips32_uClibc-0.9.33.2/usr/include/libnl-tiny -I/home/lzj/x1000_hcy/x1000_2/x1000/staging_dir/target-mipsel_mips32_uClibc-0.9.33.2/usr/include -I/home/lzj/x1000_hcy/x1000_2/x1000/staging_dir/target-mipsel_mips32_uClibc-0.9.33.2/include -I/home/lzj/x1000_hcy/x1000_2/x1000/staging_dir/toolchain-mipsel_mips32_gcc-4.6-linaro_uClibc-0.9.33.2/usr/include -I/home/lzj/x1000_hcy/x1000_2/x1000/staging_dir/toolchain-mipsel_mips32_gcc-4.6-linaro_uClibc-0.9.33.2/include -DCONFIG_LIBNL20 -D_GNU_SOURCE -Os -pipe -mno-branch-likely -mips32 -mtune=mips32 -fno-caller-saves -fhonour-copts -Wno-error=unused-but-set-variable -msoft-float -ffunction-sections -fdata-sections

define Package/libiw_nl80211
  CATEGORY:=Base system
  TITLE:=libiw_nl80211
  DEPENDS:=+libc +libnl-tiny +libnl-tiny +librt +libpthread
endef

define Build/Configure
	echo "const char iw_version[] = \"$(PKG_VERSION)\";" > $(PKG_BUILD_DIR)/version.c
	rm -f $(PKG_BUILD_DIR)/version.sh
	touch $(PKG_BUILD_DIR)/version.sh
	chmod +x $(PKG_BUILD_DIR)/version.sh
endef

TARGET_CPPFLAGS:= \
	-I$(STAGING_DIR)/usr/include/libnl-tiny \
	$(TARGET_CPPFLAGS) \
	-DCONFIG_LIBNL20 \
	-D_GNU_SOURCE

MAKE_FLAGS += \
	CFLAGS="$(TARGET_CPPFLAGS) $(TARGET_CFLAGS) -ffunction-sections -fdata-sections" \
	LDFLAGS="$(TARGET_LDFLAGS) -Wl,--gc-sections" \
	NL1FOUND="" NL2FOUND=Y \
	NLLIBNAME="libnl-tiny" \
	LIBS="-lm -lnl-tiny" \
	V=1

define Package/libiw_nl80211/description
	libiw_nl80211.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef



define Package/libiw_nl80211/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/scan_test $(1)/usr/sbin/
	$(CP) $(PKG_BUILD_DIR)/libiw_nl80211.a $(BUILD_DIR_TOOLCHAIN)/uClibc-0.9.33.2/lib/
endef

$(eval $(call BuildPackage,libiw_nl80211))

