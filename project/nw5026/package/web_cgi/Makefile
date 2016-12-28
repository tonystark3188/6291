

include $(TOPDIR)/rules.mk

PKG_NAME:=web_cgi
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk


define Package/web_cgi
  SECTION:=net
  CATEGORY:=Base system
  TITLE:=cgi program for web interface
  DEPENDS:= +libuci +libiw +udev +libwifi_uart +libm +libnl-tiny +libiw_nl80211
  
endef

define Package/web_cgi/description
  cgi program for web interface
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include -I$(KERNEL_BUILD_DIR) -I$(KERNEL_BUILD_DIR)/include 
TARGET_LDFLAGS += -L$(STAGING_DIR)/usr/lib -L$(BUILD_DIR_TOOLCHAIN)/uClibc-0.9.33.2/lib

TARGET_CFLAGS += -Wl,--gc-sections

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
	CC="$(TARGET_CC)" \
	CFLAGS="$(TARGET_CFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)" \
	all
endef

define Package/web_cgi/install
	$(INSTALL_DIR) $(1)/usr/mips
	$(INSTALL_DIR) $(1)/usr/mips/cgi-bin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/usr/mips/cgi-bin/script
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_DIR) $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/* $(1)/usr/mips/cgi-bin
	$(CP) $(PKG_BUILD_DIR)/script/* $(1)/usr/mips/cgi-bin/script
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/cfg $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/nor $(1)/bin
#	$(INSTALL_BIN) $(PKG_BUILD_DIR)/check_stm8 $(1)/bin
#	$(INSTALL_BIN) $(PKG_BUILD_DIR)/usb_chang $(1)/bin
	$(CP) ./src/encrypt_start $(1)/etc/init.d/encrypt_start
#	$(CP) $(PKG_BUILD_DIR)/bin/encrypt $(1)/bin/encrypt
#	$(CP) $(PKG_BUILD_DIR)/bin/mm $(1)/bin/mm
#	$(INSTALL_BIN) $(PKG_BUILD_DIR)/led_control_reload $(1)/bin/led_control_reload
#	$(INSTALL_BIN) $(PKG_BUILD_DIR)/led_on $(1)/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/getsysinfo $(1)/usr/mips/cgi-bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/unbind $(1)/usr/mips/cgi-bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/proccgi.sh $(1)/usr/mips/cgi-bin/
#	$(INSTALL_BIN) $(PKG_BUILD_DIR)/KeepChannel $(1)/usr/sbin/a
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/factory_reset $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/updatefw $(1)/usr/sbin/
#	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mnt_pc_dc $(1)/usr/sbin/
#	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ctl_wifi_for_3_0 $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/auto_connect $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mp_disk $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/save_wifi $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/nvram_cfg $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/erase_boot_flag $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/set_wifimode_to_boot $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/sys_upgrade $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/set_sys_flag $(1)/usr/sbin/

	$(INSTALL_BIN) ./nrender/config_nrender $(1)/bin/
	$(INSTALL_BIN) ./nrender/restore_factory_from_cfg $(1)/bin/
	$(INSTALL_BIN) ./nrender/restore_single_param $(1)/bin/
	$(INSTALL_BIN) ./nrender/samba_ushare_hotplug $(1)/bin/
	$(INSTALL_BIN) ./nrender/nrender.conf $(1)/etc/
endef

$(eval $(call BuildPackage,web_cgi))
