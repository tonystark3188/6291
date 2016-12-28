TARGET = SysInfo SysUpgrade config_param check_param set_mac StmUpgrade ArtUpgrade BootUpgrade sysupdate mm

CFG_TARGET = 	cfg nor check_stm8 KeepChannel updatefw mnt_pc_dc ctl_wifi_for_3_0 auto_connect \
				mp_disk set_wifimode_to_boot save_wifi nvram_cfg erase_boot_flag sys_upgrade set_sys_flag

LED_TARGET = led_control_reload led_on


ifdef ENCRYPT_EN
TARGET += encrypt
endif

XML_CGI_OBJ= main.o cgiget.o cgihandle.o cgipost.o uci_for_cgi.o cgiWireless.o nor_control.o

all: $(TARGET) $(CFG_TARGET) $(LED_TARGET) install

mm:mm.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
sysupdate:sysupdate.o cgiget.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
config_param:config_param.o parse_file_from_pc.o cfg_api.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
check_param:check_param.o check_file_from_pc.o cfg_api.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
SysInfo: $(XML_CGI_OBJ) nrender_conf.o cfg_api.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci -liw -ludev -lwifi_uart -liw_nl80211 -lm -lnl-tiny
SysUpgrade:SysUpgrade.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
test_uart:test_uart.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
led_control_reload:led_control_reload.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
led_on:led_on.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
cfg:cfg.o uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci
set_mac:set_mac.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
nor:nor.o nor_control.o uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci
StmUpgrade:StmUpgrade.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -lwifi_uart
ArtUpgrade:ArtUpgrade.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
BootUpgrade:BootUpgrade.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
check_stm8:check_stm8.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
KeepChannel:KeepChannel.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
encrypt:encrypt.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^	
updatefw:updatefw.o uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci
mnt_pc_dc:mnt_pc_dc.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
ctl_wifi_for_3_0:ctl_wifi_for_3_0.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
auto_connect:auto_connect.o uci_for_cgi.o cgiWireless2.o 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci -liw -liw_nl80211 -lm -lnl-tiny
mp_disk:mp_disk.o uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci
set_wifimode_to_boot:set_wifimode_to_boot.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
nvram_cfg:nvram_cfg.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
erase_boot_flag:erase_boot_flag.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
save_wifi:save_wifi.o uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci
sys_upgrade:sys_upgrade.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
set_sys_flag:set_sys_flag.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
install:
	mkdir bin
	cp -f $(TARGET) bin/
#	-cp -f $(TARGET) $(CFG_TARGET) ~

 
.PHONY: clean
clean:
	rm -f *.o $(TARGET) $(CFG_TARGET) $(LED_TARGET)

save_wifi:save_wifi.o uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci
install:
	mkdir bin
	cp -f $(TARGET) bin/
#	-cp -f $(TARGET) $(CFG_TARGET) ~

 
.PHONY: clean
clean:
	rm -f *.o $(TARGET) $(CFG_TARGET) $(LED_TARGET)

