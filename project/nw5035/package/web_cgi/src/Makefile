TARGET = letv_daemon SysInfo SysUpgrade config_param check_param set_mac StmUpgrade ArtUpgrade BootUpgrade sysupdate mm \
updatefw webupgrade search.cgi login.cgi clear.cgi nameset.cgi nameget.cgi connect.cgi  task_add.cgi \
task_version.cgi task_remove.cgi task_pause.cgi task_towait.cgi task_start.cgi task_getall.cgi task_state.cgi task_now.cgi task_getvideo.cgi album_get.cgi follow_add.cgi follow_del.cgi follow_getall.cgi file_download.cgi  wan_status.cgi wan_set.cgi \
wan_get.cgi wifi_get.cgi wifi_set.cgi file_download.cgi upgrade.cgi version.cgi task_update.cgi display getSmbPwd.cgi setSmbPwd.cgi \
storage.cgi power.cgi format.cgi online.cgi set_guest.cgi get_guest.cgi getFileNum.cgi syncTime.cgi wifi_switch.cgi upload
CFG_TARGET = 	cfg nor check_stm8 KeepChannel updatefw mnt_pc_dc ctl_wifi_for_3_0 auto_connect \
				mp_disk set_wifimode_to_boot save_wifi nvram_cfg erase_boot_flag sys_upgrade set_sys_flag

LED_TARGET = led_control_reload led_on


ifdef ENCRYPT_EN
TARGET += encrypt
endif

XML_CGI_OBJ= main.o cgiget.o cgihandle.o cgipost.o uci_for_cgi.o cgiWireless.o nor_control.o

all: $(TARGET) $(CFG_TARGET) $(LED_TARGET) install

upload:upload.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ 

getcfg:getconfig.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ 
letv_gpio:letv_gpio.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
	
display:display.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ 
wifi_switch.cgi:wifi_switch.o  uci_for_cgi.o 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci 

getFileNum.cgi:getFileNum.o msg.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ 
	
syncTime.cgi:syncTime.o uci_for_cgi.o 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci 

getSmbPwd.cgi:getSmbPwd.c uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci 

get_guest.cgi:get_guest.c uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci 	

set_guest.cgi:set_guest.c uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci 	
	
getSmbPwd.cgi:getSmbPwd.c uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci 
setSmbPwd.cgi:setSmbPwd.c uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci
	
letv_daemon:letv_daemon.o uci_for_cgi.o msg.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci

version.cgi:ota_version.o uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^  -luci

upgrade.cgi:ota_upgrade.o uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^  -luci

file_download.cgi:file_download.o uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ 

#wan_status.cgi:wan_status.o uci_for_cgi.o msg.o
#	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci

wan_status.cgi:wan_status2.o uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci #-lwifi_uart


wan_set.cgi:wan_set.o uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci

wan_get.cgi:wan_get.o uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci

wifi_get.cgi:l_getwifi.o uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci

wifi_set.cgi:l_setwifi.o uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci

task_update.cgi:task_update.o msg.o uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci
	
task_version.cgi:task_version.o msg.o uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci
	
task_getvideo.cgi:task_getvideo.o msg.o uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci

task_add.cgi:task_add.o msg.o uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci

task_remove.cgi:task_remove.o msg.o uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci
	
task_pause.cgi:task_pause.o msg.o uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci
	
task_towait.cgi:task_towait.o msg.o uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci
task_start.cgi:task_start.o msg.o uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci
task_getall.cgi:task_getall.o msg.o uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci -ljson-c
task_state.cgi:task_state.o msg.o uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci
task_now.cgi:task_now.o msg.o uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci
album_get.cgi:album_get.o msg.o uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci
follow_add.cgi:follow_add.o msg.o uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci
follow_del.cgi:follow_del.o msg.o uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci
follow_getall.cgi:follow_getall.o msg.o uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci
file_download.cgi:file_download.o msg.o uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci

search.cgi:l_scan.o cgiWireless.o uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci -liw_nl80211   -liw -lnl-tiny

#login.cgi:l_login.o uci_for_cgi.o msg.o 
#	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci

login.cgi:l_login2.o uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci -ludev #-lwifi_uart

clear.cgi:l_clear.o uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci

nameset.cgi:l_nameset.o  uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci

nameget.cgi:l_nameget.o  uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci

connect.cgi:l_connect.o  uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci

storage.cgi:l_storage.o  uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci -ludev

format.cgi:l_format.o uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci
	
power.cgi:l_getpower.o  uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci #-lwifi_uart

online.cgi:l_online.o  uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci
mm:mm.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
sysupdate:sysupdate.o cgiget.o uci_for_cgi.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -luci
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

