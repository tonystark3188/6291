/*
 * Ingenic mensa setup code
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <nand.h>
#include <net.h>
#include <netdev.h>
#include <asm/gpio.h>
#include <asm/arch/cpm.h>
#include <asm/arch/clk.h>
#include <asm/arch/mmc.h>
#include <mmc.h>

//unsigned char enter_backup_sys = 0;
unsigned char enter_sys_flag = 0;
//dm sys flag
typedef enum
{
	FIRST_SYS_FLAG = 0,
	SECOND_SYS_FLAG = 1,
}dm_sys_flag;


/*#define LED_SYS_MEM    GPIO_PB(1)  */ /* for dm6291 evb */

// #define LED_SYS    GPIO_PC(22)  /* for wendy */
#define LED_BLUE    GPIO_PC(26)  /* for 5025F, blue */
#define LED_GREEN    GPIO_PC(25)  /* for 5025F, blue */

struct cgu_clk_src cgu_clk_src[] = {
	{OTG, EXCLK},
	{LCD, MPLL},
	{MSC, MPLL},
	{SFC, MPLL},
	{CIM, MPLL},
	{PCM, MPLL},
	{I2S, EXCLK},
	{SRC_EOF,SRC_EOF}
};
extern int jz_net_initialize(bd_t *bis);
#ifdef CONFIG_BOOT_ANDROID
extern void boot_mode_select(void);
#endif

#if defined(CONFIG_CMD_BATTERYDET) && defined(CONFIG_BATTERY_INIT_GPIO)
static void battery_init_gpio(void)
{
}
#endif

int board_early_init_f(void)
{
	return 0;
}

struct private_data
{
	unsigned char wifimode;
	unsigned char mpsta;
	unsigned char boot_flag;
	unsigned char sys_flag;
};

int read_from_memory(struct private_data *p_data)
{
	ulong	addr;
	ulong bytes = 512;
	addr = 0x81000000;
	unsigned char *buf = map_sysmem(addr, bytes);
	p_data->wifimode = *buf;
	p_data->mpsta = *(buf+1);
	p_data->boot_flag = *(buf+2);
	p_data->sys_flag = *(buf+3);
	printf("wifi mode : %s\n", (p_data->wifimode == 5) ? "5G" : "2.4G");
	printf("user disk mpsta : %s\n", (p_data->mpsta == 0xaa) ? "yes" : "no");
	if(p_data->sys_flag == 0){
		printf("sys_flag = %02x\n", p_data->sys_flag);
	}
	else{
		printf("sys_flag = %02x\n", p_data->sys_flag);
	}
#ifdef BACKUP_SYSTEM
	printf("boot flag : %d\n", p_data->boot_flag);
#endif
	unmap_sysmem(buf);
	return 0;
}

int write_ulong_to_memory(int offset,unsigned long p4_sector)
{
	ulong	addr;
	ulong bytes = 512;
	addr = 0x81000000;
	unsigned char *buf = map_sysmem(addr, bytes);
	buf = buf + offset;
	
	// *((unsigned int *)buf) = (unsigned int)p4_sector;
	// *buf = 0xaa;
	// *((unsigned int *)buf) = 0x00711000;

	*buf++ = p4_sector & 0xff;
	*buf++ = (p4_sector >> 8) & 0xff;
	*buf++ = (p4_sector >> 16) & 0xff;
	*buf = (p4_sector >> 24) & 0xff;

	unmap_sysmem(buf);
	return 0;
}

int write_uchar_to_memory(int offset,unsigned char dat)
{
	ulong	addr;
	ulong bytes = 512;
	addr = 0x81000000;
	unsigned char *buf = map_sysmem(addr, bytes);
	buf = buf + offset;
	
	// *((unsigned int *)buf) = (unsigned int)p4_sector;
	// *buf = 0xaa;
	// *((unsigned int *)buf) = 0x00711000;

	*buf = dat;

	
	unmap_sysmem(buf);
	return 0;
}



#if defined(CONFIG_SPL_JZMMC_SUPPORT)
enum mmc_state {
	MMC_INVALID,
	MMC_READ,
	MMC_WRITE,
	MMC_ERASE,
};
#define WIFI_MODE_ADDR 0x40000

struct mmc *sd_mmc_init()
{
	int curr_device = -1;
	if (curr_device < 0) {
		if (get_mmc_num() > 0)
			curr_device = 0;
		else {
			puts("No MMC device available\n");
			return 1;
		}
	}
	struct mmc *mmc = find_mmc_device(curr_device);
	mmc_init(mmc);
	return mmc ;

}


int get_data_from_mmc(struct mmc *mmc,u32 blk,u32 cnt)
{
	int curr_device = 0;
	
	enum mmc_state state;
	
	state = MMC_READ;
	if (state != MMC_INVALID) {
		
		int idx = 2;
		// u32 blk, cnt, n;
		u32 n;
		void *addr;

		// if (state != MMC_ERASE) {
		// 	addr = (void *)simple_strtoul(0x80600000, NULL, 16);
		// 	++idx;
		// } else
		// 	addr = NULL;
		addr = 0x81000000;
		// blk = simple_strtoul(0x200, NULL, 16);
		// blk = 512;
		// cnt = simple_strtoul(1, NULL, 16);
		// cnt = 1;

		if (!mmc) {
			printf("no mmc device at slot %x\n", curr_device);
			return 1;
		}

		printf("\nMMC read: dev # %d, block # %d, count %d ... ",
				 curr_device, blk, cnt);

		// mmc_init(mmc);

		if ((state == MMC_WRITE || state == MMC_ERASE)) {
			if (mmc_getwp(mmc) == 1) {
				printf("Error: card is write protected!\n");
				return 1;
			}
		}

		switch (state) {
		case MMC_READ:
			n = mmc->block_dev.block_read(curr_device, blk,
						      cnt, addr);
			/* flush cache after read */
			flush_cache((ulong)addr, cnt * 512); /* FIXME */
			break;
		case MMC_WRITE:
			n = mmc->block_dev.block_write(curr_device, blk,
						      cnt, addr);
			break;
		case MMC_ERASE:
			n = mmc->block_dev.block_erase(curr_device, blk, cnt);
			break;
		default:
			BUG();
		}

		printf("%d blocks read: %s\n",
				n, (n == cnt) ? "OK" : "ERROR");
		return (n == cnt) ? 0 : 1;
	}
	return 0;
}

int write_data_to_mmc(struct mmc *mmc,u32 blk,u32 cnt)
{
	int curr_device = 0;
	enum mmc_state state;
	
	state = MMC_WRITE;
	if (state != MMC_INVALID) {
		
		int idx = 2;
		// u32 blk, cnt, n;
		u32 n;
		void *addr;

		// if (state != MMC_ERASE) {
		// 	addr = (void *)simple_strtoul(0x80600000, NULL, 16);
		// 	++idx;
		// } else
		// 	addr = NULL;
		addr = 0x81000000;
		// blk = simple_strtoul(0x200, NULL, 16);
		// blk = 512;
		// cnt = simple_strtoul(1, NULL, 16);
		// cnt = 1;

		if (!mmc) {
			printf("no mmc device at slot %x\n", curr_device);
			return 1;
		}

		printf("\nMMC write: dev # %d, block # %d, count %d ... ",
				 curr_device, blk, cnt);

		// mmc_init(mmc);

		if ((state == MMC_WRITE || state == MMC_ERASE)) {
			if (mmc_getwp(mmc) == 1) {
				printf("Error: card is write protected!\n");
				return 1;
			}
		}

		switch (state) {
		case MMC_READ:
			n = mmc->block_dev.block_read(curr_device, blk,
						      cnt, addr);
			/* flush cache after read */
			flush_cache((ulong)addr, cnt * 512); /* FIXME */
			break;
		case MMC_WRITE:
			n = mmc->block_dev.block_write(curr_device, blk,
						      cnt, addr);
			break;
		case MMC_ERASE:
			n = mmc->block_dev.block_erase(curr_device, blk, cnt);
			break;
		default:
			BUG();
		}

		printf("%d blocks write: %s\n",
				n, (n == cnt) ? "OK" : "ERROR");
		return (n == cnt) ? 0 : 1;
	}
	return 0;
}



static void print_mmcinfo(struct mmc *mmc)
{
	printf("Device: %s\n", mmc->name);
	printf("Manufacturer ID: %x\n", mmc->cid[0] >> 24);
	printf("OEM: %x\n", (mmc->cid[0] >> 8) & 0xffff);
	printf("Name: %c%c%c%c%c \n", mmc->cid[0] & 0xff,
			(mmc->cid[1] >> 24), (mmc->cid[1] >> 16) & 0xff,
			(mmc->cid[1] >> 8) & 0xff, mmc->cid[1] & 0xff);

	printf("Tran Speed: %d\n", mmc->tran_speed);
	printf("Rd Block Len: %d\n", mmc->read_bl_len);

	printf("%s version %d.%d\n", IS_SD(mmc) ? "SD" : "MMC",
			(mmc->version >> 8) & 0xf, mmc->version & 0xff);

	printf("High Capacity: %s\n", mmc->high_capacity ? "Yes" : "No");
	puts("Capacity: ");
	print_size(mmc->capacity, "\n");

	printf("Bus Width: %d-bit\n", mmc->bus_width);
}


int get_disk_size(struct mmc *mmc,unsigned long *sector)
{
	int curr_device = 0;
	unsigned long all_sector = 0;

	

	if (mmc) {
		
		puts("MMC info :\n");

		print_mmcinfo(mmc);
		all_sector = (unsigned long)((mmc->capacity) >>9 ) ;
		*sector = all_sector;
		printf("all sectors : %lu\n", all_sector);
		
		return 0;
	} else {
		printf("no mmc device at slot %x\n", curr_device);
		return 1;
	}
}


int mp_disk(struct mmc *mmc)
{
	unsigned long p4_sector;
	unsigned long p4_offset;
	// unsigned long i;
	// char tmp_str[16]="\0";
	// strncpy(tmp_str,CONFIG_MBR_P3_OFF,strlen(CONFIG_MBR_P3_OFF)-2);
	// i = atoi(tmp_str);
	// printf("p4_offset = %d\n", i);
	p4_offset = CONFIG_MBR_P3_OFF_INT*1024*2;
	get_disk_size(mmc,&p4_sector);
	p4_sector = p4_sector - p4_offset ;
	printf("p4 sectors : %lu %08x\n\n",p4_sector,p4_sector);
	get_data_from_mmc(mmc,0,1);
	write_ulong_to_memory(0x1fa,p4_sector);
	write_data_to_mmc(mmc,0,1);

	get_data_from_mmc(mmc,512,1);
	write_uchar_to_memory(1,0xaa);
	write_data_to_mmc(mmc,512,1);

	return 0;
}

#ifdef FIX_MBR

#define ARRAY_SIZE(x)		((sizeof(x))/(sizeof(x[0])))

#define DBR_HEAD_1		0xEB
#define DBR_HEAD_3		0x90

#define DBR_TAIL_1		0x55
#define DBR_TAIL_2		0xAA

#define P1_FS			0x1C2
#define P1_OFFSET		0x1C6
#define P1_SIZE			0x1Ca

#define P2_FS			0x1D2
#define P2_OFFSET		0x1D6
#define P2_SIZE			0x1Da

#define P3_FS			0x1E2
#define P3_OFFSET		0x1E6
#define P3_SIZE			0x1Ea

#define P4_FS			0x1F2
#define P4_OFFSET		0x1F6
#define P4_SIZE			0x1Fa


struct mbr_tab_item {
	unsigned int	offset;
	unsigned int	size;
	unsigned char	type;
};

struct mbr_head {
	unsigned char head1;
	unsigned char head2;
	unsigned char head3;
};

struct mbr_tail {
	unsigned char tail1;
	unsigned char tail2;
};

int read_mbr_from_memory(struct mbr_tab_item *p_tab_item_mbr, struct mbr_head *p_head_mbr, struct mbr_tail *p_tail_mbr)
{
	int i = 0;
	ulong	addr;
	ulong bytes = 512;
	addr = 0x81000000;
	unsigned char *buf = map_sysmem(addr, bytes);

	if(NULL != p_tab_item_mbr){
		memcpy(&(p_tab_item_mbr)[0].offset,buf+P1_OFFSET,sizeof(unsigned int));
		memcpy(&(p_tab_item_mbr)[1].offset,buf+P2_OFFSET,sizeof(unsigned int));
		memcpy(&(p_tab_item_mbr)[2].offset,buf+P3_OFFSET,sizeof(unsigned int));
		memcpy(&(p_tab_item_mbr)[3].offset,buf+P4_OFFSET,sizeof(unsigned int));
		
		memcpy(&(p_tab_item_mbr)[0].size,buf+P1_SIZE,sizeof(unsigned int));
		memcpy(&(p_tab_item_mbr)[1].size,buf+P2_SIZE,sizeof(unsigned int));
		memcpy(&(p_tab_item_mbr)[2].size,buf+P3_SIZE,sizeof(unsigned int));
		memcpy(&(p_tab_item_mbr)[3].size,buf+P4_SIZE,sizeof(unsigned int));

		memcpy(&(p_tab_item_mbr)[0].type,buf+P1_FS,sizeof(unsigned char));
		memcpy(&(p_tab_item_mbr)[1].type,buf+P2_FS,sizeof(unsigned char));
		memcpy(&(p_tab_item_mbr)[2].type,buf+P3_FS,sizeof(unsigned char));
		memcpy(&(p_tab_item_mbr)[3].type,buf+P4_FS,sizeof(unsigned char));
	#if 0
		printf("partition\ttype\toffset\t\tsize\n");
		for(i=0;i<4;i++){
			printf("%d\t\t%02x\t%08x(%d MB)\t%08x(%d MB)\n",i+1,(p_tab_item_mbr)[i].type,(p_tab_item_mbr)[i].offset,
					((p_tab_item_mbr)[i].offset)/(1024*2),(p_tab_item_mbr)[i].size,((p_tab_item_mbr)[i].size)/(1024*2));

		}
		printf("\n");
	#endif
	}

	if(NULL != p_head_mbr){
		memcpy(&(*p_head_mbr).head1,buf+0,sizeof(unsigned char));
		memcpy(&(*p_head_mbr).head2,buf+1,sizeof(unsigned char));
		memcpy(&(*p_head_mbr).head3,buf+2,sizeof(unsigned char));
	#if 1
		printf("dbr head : ");
			printf("%02x %02x %02x\n", (*p_head_mbr).head1, (*p_head_mbr).head2, (*p_head_mbr).head3);
	#endif
	}

	if(NULL != p_tail_mbr){
		memcpy(&(*p_tail_mbr).tail1,buf+0x1FE,sizeof(unsigned char));
		memcpy(&(*p_tail_mbr).tail2,buf+0x1FF,sizeof(unsigned char));
	#if 1
		printf("dbr tail : ");
			printf("%02x %02x\n", (*p_tail_mbr).tail1, (*p_tail_mbr).tail2);
	#endif
	}
	
	unmap_sysmem(buf);
	return 0;
}

int fix_mbr(struct mmc *mmc)
{
	int ret = 0;
	struct mbr_tab_item	tab_item_mbr1[4];
	struct mbr_tab_item	tab_item_mbr2[4];
	struct mbr_head dm_head_mbr2;
	struct mbr_tail dm_tail_mbr2;
	struct mbr_head dm_head_dbr;
	struct mbr_tail dm_tail_dbr;
	memset(tab_item_mbr1, 0, ARRAY_SIZE(tab_item_mbr1));
	memset(tab_item_mbr2, 0, ARRAY_SIZE(tab_item_mbr2));
	memset(&dm_head_mbr2, 0, sizeof(struct mbr_head));
	memset(&dm_tail_mbr2, 0, sizeof(struct mbr_tail));
	memset(&dm_head_dbr, 0, sizeof(struct mbr_head));
	memset(&dm_tail_dbr, 0, sizeof(struct mbr_tail));

	//读取MBR1信息
	ret = get_data_from_mmc(mmc,0,1);
	if(ret != 0){
		printf("get data form mmc error\n");
		return -1;
	}
	ret = read_mbr_from_memory(tab_item_mbr1, NULL, NULL);
	if(ret != 0){
		printf("read data form memory error\n");
		return -1;
	}

	//读取第四个分区信息
	//printf("get mbr2 tab_item_mbr1[3].offset = %08x\n", tab_item_mbr1[3].offset);
	ret = get_data_from_mmc(mmc,tab_item_mbr1[3].offset,1);
	if(ret != 0){
		printf("get data form mmc error\n");
		return -1;
	}
	ret = read_mbr_from_memory(tab_item_mbr2, &dm_head_mbr2, &dm_tail_mbr2);
	if(ret != 0){
		printf("read data form memory error\n");
		return -1;
	}

	//判断第四个分区首512字节是否是MBR
	if((dm_head_mbr2.head1 == DBR_HEAD_1) && (dm_head_mbr2.head3 == DBR_HEAD_3) && 
		(dm_tail_mbr2.tail1 == DBR_TAIL_1) && (dm_tail_mbr2.tail2 == DBR_TAIL_2)){
		printf("no seconf mbr\n");
		return 1;
	}
	else if((dm_tail_mbr2.tail1 != DBR_TAIL_1) && (dm_tail_mbr2.tail2 != DBR_TAIL_2)){
		printf("no seconf mbr or dbr\n");
		return 1;
	}else{
		printf("has second mbr\n");
	}

	//如果为MBR，判断分区偏移地址是否为DBR
	unsigned int dbr_offset = tab_item_mbr1[3].offset + tab_item_mbr2[0].offset;
	ret = get_data_from_mmc(mmc,dbr_offset,1);
	if(ret != 0){
		printf("get data form mmc error\n");
		return -1;
	}
	ret = read_mbr_from_memory(NULL, &dm_head_dbr, &dm_tail_dbr);
	if(ret != 0){
		printf("read data form memory error\n");
		return -1;
	}

	//判断是否为DBR
	if((dm_head_dbr.head1 == DBR_HEAD_1) && (dm_head_dbr.head3 == DBR_HEAD_3) && 
		(dm_tail_dbr.tail1 == DBR_TAIL_1) && (dm_tail_dbr.tail2 == DBR_TAIL_2)){
		printf("has dbr\n");
	}
	else{
		printf("no dbr\n");
		return 1;
	}

	//更新MBR信息
	ret = get_data_from_mmc(mmc,0,1);
	if(ret != 0){
		printf("get data form mmc error\n");
		return -1;
	}
	write_uchar_to_memory(P4_FS, tab_item_mbr2[0].type);
	write_ulong_to_memory(P4_OFFSET, dbr_offset);
	write_ulong_to_memory(P4_SIZE, tab_item_mbr2[0].size);
	write_data_to_mmc(mmc,0,1);
	
	return 0;
}

#endif
#endif

int led_init(unsigned char wifimode)
{
	int sys_led;
	int off_led;

	if(wifimode == 5)
	{
		sys_led = LED_GREEN;
		off_led = LED_BLUE;
	}
	else
	{
		sys_led = LED_BLUE;
		off_led = LED_GREEN;
	}

	gpio_request(off_led , "off-led");
	gpio_direction_output(off_led , 0);
	gpio_set_value(off_led, 1);
	
	gpio_request(sys_led , "led-sys");
	gpio_direction_output(sys_led , 0);
	gpio_set_value(sys_led, 0);
	udelay(200000);
	gpio_set_value(sys_led, 1);
	udelay(200000);
	gpio_set_value(sys_led, 0);
	udelay(200000);
	gpio_set_value(sys_led, 1);
	udelay(200000);
	gpio_set_value(sys_led, 0);
/*	udelay(200000);
	gpio_set_value(sys_led, 1);
*/
}

#ifdef CONFIG_USB_GADGET
int jz_udc_probe(void);
void board_usb_init(void)
{
	printf("USB_udc_probe\n");
	jz_udc_probe();
}
#endif /* CONFIG_USB_GADGET */

int misc_init_r(void)
{
	struct private_data p_data;
	memset(&p_data, 0, sizeof(struct private_data));
#if defined(CONFIG_SPL_JZMMC_SUPPORT)
	struct mmc *mmc=sd_mmc_init();
	get_data_from_mmc(mmc,512,1);
	read_from_memory(&p_data);
	led_init(p_data.wifimode);
	if(p_data.mpsta != 0xaa)
		mp_disk(mmc);
#endif

#ifdef BACKUP_SYSTEM
	#if defined(CONFIG_SPL_JZMMC_SUPPORT)
	if(p_data.boot_flag >= 3 && p_data.boot_flag != 0xff)
	{
		//enter_backup_sys = 1;
		if(p_data.boot_flag == 3){
			get_data_from_mmc(mmc,512,1);
			write_uchar_to_memory(2,p_data.boot_flag + 1);
			write_data_to_mmc(mmc,512,1);
		}
		if(p_data.sys_flag == 0){
			enter_sys_flag = SECOND_SYS_FLAG;
		}
		else{
			enter_sys_flag = FIRST_SYS_FLAG;
		}
		printf("backup system mode.\n");
		if(enter_sys_flag == FIRST_SYS_FLAG)
			printf("we will enter to the first system\n");
		else
			printf("we will enter to the second system\n");
	}
	else
	{
		//enter_backup_sys = 0;
		if(p_data.boot_flag == 0xff)
		{
			get_data_from_mmc(mmc,512,1);
			write_uchar_to_memory(2,0);
			write_data_to_mmc(mmc,512,1);
		}
		else
		{
			get_data_from_mmc(mmc,512,1);
			write_uchar_to_memory(2,p_data.boot_flag + 1);
			write_data_to_mmc(mmc,512,1);
		}

		if(p_data.sys_flag == 0){
			enter_sys_flag = FIRST_SYS_FLAG;
		}
		else{
			enter_sys_flag = SECOND_SYS_FLAG;
		}

		printf("normal system mode.\n");
		if(enter_sys_flag == FIRST_SYS_FLAG)
			printf("we will enter to the first system\n");
		else
			printf("we will enter to the second system\n");
		
	}
	#endif
#endif

#ifdef FIX_MBR
	int ret = 0;
	ret = fix_mbr(mmc);
	if(ret == 1)
	{
		printf("no need fix mbr!\n");
	}
	else if(ret == 0){
		printf("fix mbr success!\n");
	}
	else{
		printf("fix mbr fail!\n");
	}
#endif
	
#if 0 /* TO DO */
	uint8_t mac[6] = { 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc };

	/* set MAC address */
	eth_setenv_enetaddr("ethaddr", mac);
#endif
#ifdef CONFIG_BOOT_ANDROID
	boot_mode_select();
#endif

#if defined(CONFIG_CMD_BATTERYDET) && defined(CONFIG_BATTERY_INIT_GPIO)
	battery_init_gpio();
#endif
	return 0;
}

#ifdef CONFIG_MMC
int board_mmc_init(bd_t *bd)
{
	jz_mmc_init();
	return 0;
}
#endif

int board_eth_init(bd_t *bis)
{
	int rv;
#ifndef  CONFIG_USB_ETHER
	/* reset grus DM9000 */
#ifdef CONFIG_NET_GMAC
	rv = jz_net_initialize(bis);
#endif
#else
	rv = usb_eth_initialize(bis);
#endif
	return rv;
}

#ifdef CONFIG_SPL_NOR_SUPPORT
int spl_start_uboot(void)
{
	return 1;
}
#endif

/* U-Boot common routines */
int checkboard(void)
{

	puts("Board: Canna (Ingenic XBurst X1000 SoC)\n");
	printf("DDR Freq: %d\n",CONFIG_SYS_MEM_FREQ);
	printf("version: new\n");
	return 0;
}

#ifdef CONFIG_SPL_BUILD

void spl_board_init(void)
{
}

#endif /* CONFIG_SPL_BUILD */
