#
# Copyright (C) 2009-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

JFFS2_BLOCKSIZE=32k 512k

ifneq ($(CONFIG_TARGET_xburst_qi_lb60),)
UBI_OPTS = -m 4096 -p 512KiB
UBIFS_OPTS = -m 4096 -e 516096 -c 4095
else
UBI_OPTS = -m 2048 -p 128KiB -s 512
UBIFS_OPTS = -m 2048 -e 126KiB -c 4096
endif

UIMAGE:=$(BIN_DIR)/$(IMG_PREFIX)-uImage.bin

define kernel_entry
0x$(shell $(TARGET_CROSS)nm $(1) 2>/dev/null | grep " kernel_entry" | cut -f1 -d ' ')
endef


define MkImageGzip
	gzip -9 -c $(1) > $(1).gz
	mkimage -A mips -O linux -T kernel -a 0x80010000 -C gzip \
		-e $(call kernel_entry,$(KDIR)/vmlinux.debug) -n 'MIPS OpenWrt Linux-$(LINUX_VERSION)' \
		-d $(1).gz $(2)
endef

define Image/BuildKernel
	$(call MkImageGzip,$(KDIR)/vmlinux,$(UIMAGE))
endef

define Image/Build/squashfs
	$(call prepare_generic_squashfs,$(KDIR)/root.squashfs)
endef

define Image/Build
	dd if=$(KDIR)/root.$(1) of=$(BIN_DIR)/$(IMG_PREFIX)-root.$(1) bs=128k conv=sync
ifeq ($(1),ext4)
	dd if=$(KDIR)/factory.$(1) of=$(BIN_DIR)/$(IMG_PREFIX)-factory.$(1) bs=128k conv=sync
	dd if=$(KDIR)/user.$(1) of=$(BIN_DIR)/$(IMG_PREFIX)-user.$(1) bs=128k conv=sync
#	dd if=$(KDIR)/empty.$(1) of=$(BIN_DIR)/$(IMG_PREFIX)-empty.$(1) bs=128k conv=sync
	
	-rm -f $(BIN_DIR)/6291-update-fw.bin $(BIN_DIR)/6291-update-fw-gz.bin
	
	tr '\000' '\377' < /dev/zero | dd of=$(BIN_DIR)/kernel-5MB-0xff.bin bs=1024 count=5120 status=noxfer
	dd if=$(UIMAGE) of=$(BIN_DIR)/kernel-5MB-0xff.bin conv=notrunc status=noxfer

	cat $(BIN_DIR)/kernel-5MB-0xff.bin $(BIN_DIR)/$(IMG_PREFIX)-root.$(1) >$(BIN_DIR)/6291-update-fw.bin

	gzip -9 -c $(BIN_DIR)/6291-update-fw.bin > $(BIN_DIR)/6291-update-fw-gz.bin

	rm -f $(BIN_DIR)/kernel-5MB-0xff.bin



endif
endef

$(eval $(call BuildImage))
