#
# Copyright (C) 2009-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk
PRODUCT_MODEL=nw5025F_ac

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
		-e $(call kernel_entry,$(KDIR)/vmlinux.debug) -n '$(PRODUCT_MODEL)' \
		-d $(1).gz $(2)
endef

define MkImageLzma
	lzma e $(1) -lc1 -lp2 -pb2 $(1).lzma
	mkimage -A mips -O linux -T kernel -a 0x80010000 -C lzma \
		-e $(call kernel_entry,$(KDIR)/vmlinux.debug) -n '$(PRODUCT_MODEL)' \
		-d $(1).lzma $(2)
endef

define Image/BuildKernel
	$(call MkImageGzip,$(KDIR)/vmlinux,$(UIMAGE))
	$(call MkImageLzma,$(KDIR)/vmlinux,$(UIMAGE).lzma)
endef

define Image/Build/squashfs
	$(call prepare_generic_squashfs,$(KDIR)/root.squashfs)
endef

define Image/Build
	dd if=$(KDIR)/root.$(1) of=$(BIN_DIR)/$(IMG_PREFIX)-root.$(1) bs=128k conv=sync
ifeq ($(1),ext4)
	dd if=$(KDIR)/factory.$(1) of=$(BIN_DIR)/$(IMG_PREFIX)-factory.$(1) bs=128k conv=sync
#	dd if=$(KDIR)/user.$(1) of=$(BIN_DIR)/$(IMG_PREFIX)-user.$(1) bs=128k conv=sync
#	dd if=$(KDIR)/empty.$(1) of=$(BIN_DIR)/$(IMG_PREFIX)-empty.$(1) bs=128k conv=sync
	
	-rm -f $(BIN_DIR)/6291-update-fw.bin $(BIN_DIR)/6291-sd-burn-128MB.img
	
	-rm -f $(BIN_DIR)/6291-update-fw.bin.md5 $(BIN_DIR)/6291-update-fw-gz.bin
	tr '\000' '\377' < /dev/zero | dd of=$(BIN_DIR)/kernel-5MB-0xff.bin bs=1024 count=5120 status=noxfer
	dd if=$(UIMAGE) of=$(BIN_DIR)/kernel-5MB-0xff.bin conv=notrunc status=noxfer

	cat $(BIN_DIR)/kernel-5MB-0xff.bin $(BIN_DIR)/$(IMG_PREFIX)-root.$(1) >$(BIN_DIR)/6291-update-fw.bin

	md5sum $(BIN_DIR)/6291-update-fw.bin >$(BIN_DIR)/6291-update-fw.bin.md5

	cd ${BIN_DIR} ; tar -czvf 6291-update-fw-gz.bin 6291-update-fw.bin 6291-update-fw.bin.md5 ; cd -
	md5sum $(BIN_DIR)/6291-update-fw-gz.bin >$(BIN_DIR)/6291-update-fw-gz.bin.md5


#	tr '\000' '\377' < /dev/zero | dd of=$(BIN_DIR)/6291-sd-burn-64MB.img bs=1024 count=65536 status=noxfer
#	dd if=$(TOPDIR)/uboot/uboot-6291-sdboot.bin of=$(BIN_DIR)/6291-sd-burn-64MB.img conv=notrunc status=noxfer
#	dd if=$(BIN_DIR)/6291-update-fw.bin of=$(BIN_DIR)/6291-sd-burn-64MB.img bs=1M seek=3 conv=notrunc status=noxfer
#	dd if=$(BIN_DIR)/$(IMG_PREFIX)-factory.$(1) of=$(BIN_DIR)/6291-sd-burn-64MB.img bs=1M seek=61 conv=notrunc status=noxfer
#	dd if=$(BIN_DIR)/$(IMG_PREFIX)-user.$(1) of=$(BIN_DIR)/6291-sd-burn-64MB.img bs=1M seek=63 conv=notrunc status=noxfer

	tr '\000' '\377' < /dev/zero | dd of=$(BIN_DIR)/6291-sd-burn-128MB.img bs=1024 count=131072 status=noxfer
	dd if=$(TOPDIR)/uboot/uboot-6291-sdboot.bin of=$(BIN_DIR)/6291-sd-burn-128MB.img conv=notrunc status=noxfer
	dd if=$(BIN_DIR)/kernel-5MB-0xff.bin of=$(BIN_DIR)/6291-sd-burn-128MB.img bs=1M seek=3 conv=notrunc status=noxfer
	dd if=$(BIN_DIR)/kernel-5MB-0xff.bin of=$(BIN_DIR)/6291-sd-burn-128MB.img bs=1M seek=8 conv=notrunc status=noxfer
	dd if=$(BIN_DIR)/$(IMG_PREFIX)-root.$(1) of=$(BIN_DIR)/6291-sd-burn-128MB.img bs=1M seek=13 conv=notrunc status=noxfer
	dd if=$(BIN_DIR)/$(IMG_PREFIX)-root.$(1) of=$(BIN_DIR)/6291-sd-burn-128MB.img bs=1M seek=70 conv=notrunc status=noxfer
	dd if=$(BIN_DIR)/$(IMG_PREFIX)-factory.$(1) of=$(BIN_DIR)/6291-sd-burn-128MB.img bs=1M seek=127 conv=notrunc status=noxfer

	md5sum $(BIN_DIR)/6291-sd-burn-128MB.img >$(BIN_DIR)/6291-sd-burn-128MB.img.md5

	
	rm -f $(BIN_DIR)/kernel-5MB-0xff.bin



endif
endef

$(eval $(call BuildImage))
