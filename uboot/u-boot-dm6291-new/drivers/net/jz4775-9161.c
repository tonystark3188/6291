#include <config.h>
#include <common.h>
#include <malloc.h>
#include <net.h>
#include <command.h>
#include <asm/io.h>
#include <asm/cache.h>

#include "SynopGMAC_Dev.h"

/* The amount of time between FLP bursts is 16ms +/- 8ms */
#define MAX_WAIT	40000

static synopGMACdevice	*gmacdev;
static synopGMACdevice	_gmacdev;

#define NUM_RX_DESCS	PKTBUFSRX
#define NUM_TX_DESCS	4

static DmaDesc	_tx_desc[NUM_TX_DESCS];
static DmaDesc	_rx_desc[NUM_RX_DESCS];
static DmaDesc	*tx_desc;
static DmaDesc	*rx_desc;
static int	next_tx;
static int	next_rx;

unsigned char tx_buff[NUM_TX_DESCS * 2048];

__attribute__((__unused__)) static void jzmac_dump_dma_desc2(DmaDesc *desc)
{
	printf("desc: %p, status: 0x%08x buf1: 0x%08x len: %u\n",
	       desc, desc->status, desc->buffer1, desc->length);
}
__attribute__((__unused__)) static void jzmac_dump_rx_desc(void)
{
	int i = 0;
	printf("\n===================rx====================\n");
	for (i = 0; i < NUM_RX_DESCS; i++) {
		jzmac_dump_dma_desc2(rx_desc + i);
	}
	printf("\n=========================================\n");
}
__attribute__((__unused__)) static void jzmac_dump_tx_desc(void)
{
	int i = 0;
	printf("\n===================tx====================\n");
	for (i = 0; i < NUM_TX_DESCS; i++) {
		jzmac_dump_dma_desc2(tx_desc + i);
	}
	printf("\n=========================================\n");
}
__attribute__((__unused__)) static void jzmac_dump_all_desc(void)
{
	jzmac_dump_rx_desc();
	jzmac_dump_tx_desc();
}
__attribute__((__unused__)) static void jzmac_dump_pkt_data(unsigned char *data, int len)
{
	int i = 0;
	printf("\t0x0000: ");
	for (i = 0; i < len; i++) {
		printf("%02x ", data[i]);

		if ((i % 8) == 7)
			printf(" ");

		if ( (i != 0) && ((i % 16) == 15) )
			printf("\n\t0x%04x: ", i+1);
	}
	printf("\n");
}
__attribute__((__unused__)) static void jzmac_dump_arp_reply(unsigned char *data, int len)
{
	int i = 0;

	for (i = 0; i < 6; i++) {
		if (data[i] != 0xff)
			break;
	}

	if (i == 6)  // broadcast pkt
		return;

	if ( (data[12] == 0x08) && (data[13] == 0x06) && (data[20] == 0x00)) {
		jzmac_dump_pkt_data(data, len);
	}
}
__attribute__((__unused__)) static void jzmac_dump_icmp_reply(unsigned char *data, int len)
{
	if ( (data[12] == 0x08) && (data[13] == 0x00) &&
	     (data[23] == 0x01) && (data[34] == 0x00) ) {
		jzmac_dump_pkt_data(data, len);
	}
}

//static u32 full_duplex, phy_mode;

struct jzmac_reg
{
	u32    addr;
	char   * name;
};

static struct jzmac_reg mac[] =
{
	{ 0x0000, "                  Config" },
	{ 0x0004, "            Frame Filter" },
	{ 0x0008, "             MAC HT High" },
	{ 0x000C, "              MAC HT Low" },
	{ 0x0010, "               GMII Addr" },
	{ 0x0014, "               GMII Data" },
	{ 0x0018, "            Flow Control" },
	{ 0x001C, "                VLAN Tag" },
	{ 0x0020, "            GMAC Version" },
	{ 0x0024, "            GMAC Debug  " },
	{ 0x0028, "Remote Wake-Up Frame Filter" },
	{ 0x002C, "  PMT Control and Status" },
	{ 0x0030, "  LPI Control and status" },
	{ 0x0034, "      LPI Timers Control" },
	{ 0x0038, "        Interrupt Status" },
	{ 0x003c, "        Interrupt Mask" },
	{ 0x0040, "          MAC Addr0 High" },
	{ 0x0044, "           MAC Addr0 Low" },
	{ 0x0048, "          MAC Addr1 High" },
	{ 0x004c, "           MAC Addr1 Low" },
	{ 0x0100, "           MMC Ctrl Reg " },
	{ 0x010c, "        MMC Intr Msk(rx)" },
	{ 0x0110, "        MMC Intr Msk(tx)" },
	{ 0x0200, "    MMC Intr Msk(rx ipc)" },
	{ 0x0738, "          AVMAC Ctrl Reg" },
	{ 0, 0 }
};
static struct jzmac_reg dma0[] =
{
	{ 0x0000, "[CH0] CSR0   Bus Mode" },
	{ 0x0004, "[CH0] CSR1   TxPlDmnd" },
	{ 0x0008, "[CH0] CSR2   RxPlDmnd" },
	{ 0x000C, "[CH0] CSR3    Rx Base" },
	{ 0x0010, "[CH0] CSR4    Tx Base" },
	{ 0x0014, "[CH0] CSR5     Status" },
	{ 0x0018, "[CH0] CSR6    Control" },
	{ 0x001C, "[CH0] CSR7 Int Enable" },
	{ 0x0020, "[CH0] CSR8 Missed Fr." },
	{ 0x0024, "[CH0] Recv Intr Wd.Tm." },
	{ 0x0028, "[CH0] AXI Bus Mode   " },
	{ 0x002c, "[CH0] AHB or AXI Status" },
	{ 0x0048, "[CH0] CSR18 Tx Desc  " },
	{ 0x004C, "[CH0] CSR19 Rx Desc  " },
	{ 0x0050, "[CH0] CSR20 Tx Buffer" },
	{ 0x0054, "[CH0] CSR21 Rx Buffer" },
	{ 0x0058, "CSR22 HWCFG          " },
	{ 0, 0 }
};

__attribute__((__unused__)) static void jzmac_dump_dma_regs(const char *func, int line)
{
	struct jzmac_reg *reg = dma0;

	printf("======================DMA Regs start===================\n");
	while(reg->name) {
		printf("%s:\t0x%08x\n", reg->name,
		       synopGMACReadReg((u32 *)gmacdev->DmaBase,reg->addr));
		reg++;
	}
	printf("======================DMA Regs end===================\n");
}

__attribute__((__unused__)) static void jzmac_dump_mac_regs(const char *func, int line)
{
	struct jzmac_reg *reg = mac;

	printf("======================MAC Regs start===================\n");
	while(reg->name) {
		printf("%s:\t0x%08x\n", reg->name,
		       synopGMACReadReg((u32 *)gmacdev->MacBase,reg->addr));
		reg++;
	}
	printf("======================MAC Regs end===================\n");
}

__attribute__((__unused__)) static void jzmac_dump_all_regs(const char *func, int line)
{
	jzmac_dump_dma_regs(func, line);
	jzmac_dump_mac_regs(func, line);
}

static void jzmac_init(void) {
	synopGMAC_wd_disable(gmacdev);
	synopGMAC_jab_disable(gmacdev);
	//synopGMAC_frame_burst_enable(gmacdev);
	synopGMAC_jumbo_frame_enable(gmacdev);
	synopGMAC_rx_own_disable(gmacdev);
	synopGMAC_loopback_off(gmacdev);
	/* default to full duplex, I think this will be the common case */
	synopGMAC_set_full_duplex(gmacdev);
	synopGMAC_retry_enable(gmacdev);
	synopGMAC_pad_crc_strip_disable(gmacdev);
	synopGMAC_back_off_limit(gmacdev,GmacBackoffLimit0);
	synopGMAC_deferral_check_disable(gmacdev);
#if 0
	synopGMAC_tx_enable(gmacdev);
	synopGMAC_rx_enable(gmacdev);
#endif
	/* default to 100M, I think this will be the common case */
	synopGMAC_select_mii(gmacdev);

	/*Frame Filter Configuration*/
	synopGMAC_frame_filter_enable(gmacdev);
	synopGMAC_set_pass_control(gmacdev,GmacPassControl0);
	synopGMAC_broadcast_enable(gmacdev);
	//synopGMAC_src_addr_filter_disable(gmacdev);
	synopGMAC_src_addr_filter_enable(gmacdev);
	synopGMAC_multicast_disable(gmacdev);
	synopGMAC_dst_addr_filter_normal(gmacdev);
	synopGMAC_multicast_hash_filter_disable(gmacdev);
	synopGMAC_promisc_disable(gmacdev);
	synopGMAC_unicast_hash_filter_disable(gmacdev);

	/*Flow Control Configuration*/
	//synopGMAC_unicast_pause_frame_detect_disable(gmacdev);
	synopGMAC_unicast_pause_frame_detect_enable(gmacdev);
	synopGMAC_rx_flow_control_enable(gmacdev);
	//synopGMAC_rx_flow_control_disable(gmacdev);
	//synopGMAC_tx_flow_control_disable(gmacdev);
	synopGMAC_tx_flow_control_enable(gmacdev);
}
static void jz47xx_mac_configure(void)
{
	/* pbl32 incr with rxthreshold 128 and Desc is 8 Words */
	synopGMAC_dma_bus_mode_init(gmacdev,
				    DmaBurstLength32 | DmaDescriptorSkip2 |
				    DmaDescriptor8Words | DmaFixedBurstEnable |
					0x02000000);
	synopGMAC_dma_control_init(gmacdev,
				   DmaStoreAndForward | DmaTxSecondFrame |
				   DmaRxThreshCtrl128);

	/* Initialize the mac interface */
	jzmac_init();

	/* This enables the pause control in Full duplex mode of operation */
	//synopGMAC_pause_control(gmacdev);
	synopGMAC_clear_interrupt(gmacdev);

	/*
	 * Disable the interrupts generated by MMC and IPC counters.
	 * If these are not disabled ISR should be modified accordingly
	 * to handle these interrupts.
	*/
	synopGMAC_disable_mmc_tx_interrupt(gmacdev, 0xFFFFFFFF);
	synopGMAC_disable_mmc_rx_interrupt(gmacdev, 0xFFFFFFFF);
	synopGMAC_disable_mmc_ipc_rx_interrupt(gmacdev, 0xFFFFFFFF);
}

/***************************************************************************
 * ETH interface routines
 **************************************************************************/

static void jzmac_restart_tx_dma(void)
{
	u32 data;

	/* TODO: clear error status bits if any */

	data = synopGMACReadReg((u32 *)gmacdev->DmaBase, DmaControl);
	if (data & DmaTxStart) {
		synopGMAC_resume_dma_tx(gmacdev);
	} else {
		synopGMAC_enable_dma_tx(gmacdev);
	}
}

static int jz_send(struct eth_device* dev, void *packet, int length)
{
	DmaDesc *desc = tx_desc + next_tx;
	int ret = 1;
	int wait_delay = 1000;

	if (!packet) {
		printf("jz_send: packet is NULL !\n");
		return -1;
	}

	memset(&tx_buff[next_tx * 2048], 0, 2048);
	memcpy((void *)&tx_buff[next_tx * 2048], packet, length);
	flush_dcache_all();

	/* prepare DMA data */
	desc->length |= (((length <<DescSize1Shift) & DescSize1Mask)
			 | ((0 <<DescSize2Shift) & DescSize2Mask));

	desc->buffer1 = virt_to_phys(&tx_buff[next_tx * 2048]);
//	desc->buffer1 = virt_to_phys(packet);
	/* ENH_DESC */
	desc->status |=  (DescTxFirst | DescTxLast | DescTxIntEnable);
	desc->status |= DescOwnByDma;

//	flush_dcache_all();

	/* start tx operation*/
	jzmac_restart_tx_dma();

	/* wait until current desc transfer done */
#if 1
	while(--wait_delay && synopGMAC_is_desc_owned_by_dma(desc)) {
		udelay(100);
	}
	/* check if there is error during transmit */
	if(wait_delay == 0)
	{
		printf("error may happen, need reload\n");
		return -1;
	}
#endif

//	printf("send data length: %d\n", length);

//	jzmac_dump_dma_regs(__func__, __LINE__);
	/* if error occurs, then handle the error */

	next_tx++;
	if (next_tx >= NUM_TX_DESCS)
		next_tx = 0;

	return ret;
}

static int jz_recv(struct eth_device* dev)
{
	volatile DmaDesc *desc;
	int length = -1;

	desc = rx_desc + next_rx;

	if(!synopGMAC_is_desc_owned_by_dma(desc)) {

		/* since current desc contains the valid data, now we're going to get the data */
		length = synopGMAC_get_rx_desc_frame_length(desc->status);
		/* Pass the packet up to the protocol layers */

#if 0
		jzmac_dump_arp_reply(NetRxPackets[next_rx], length - 4);
		jzmac_dump_icmp_reply(NetRxPackets[next_rx], length - 4);
#endif

		// printf("recv length:%d\n", length);
		//jzmac_dump_dma_regs(__func__, __LINE__);
#if 1
		if(length  < 28) {
			udelay(100);
			return -1;
		}
#endif
		NetReceive(NetRxPackets[next_rx], length - 4);
		/* after got data, make sure the dma owns desc to recv data from MII */
		desc->status = DescOwnByDma;

		synopGMAC_resume_dma_rx(gmacdev);

		flush_dcache_all();

		next_rx++;
		if (next_rx >= NUM_RX_DESCS)
			next_rx = 0;
	}

	return length;
}

static int jz_init(struct eth_device* dev, bd_t * bd)
{
	int i;
	/* int phy_id; */
	next_tx = 0;
	next_rx = 0;

	memset(tx_buff, 0, 2048 * NUM_TX_DESCS);

	printf("jz_init......\n");
	/* init global pointers */
	tx_desc = (DmaDesc *)((unsigned long)_tx_desc | 0xa0000000);
	rx_desc = (DmaDesc *)((unsigned long)_rx_desc | 0xa0000000);

	/* reset GMAC, prepare to search phy */
	synopGMAC_reset(gmacdev);

	/* we do not process interrupts */
	synopGMAC_disable_interrupt_all(gmacdev);

	// Set MAC address
	synopGMAC_set_mac_addr(gmacdev,
			       GmacAddr0High,GmacAddr0Low,
			       eth_get_dev()->enetaddr);

	synopGMAC_set_mdc_clk_div(gmacdev,GmiiCsrClk2);
	gmacdev->ClockDivMdc = synopGMAC_get_mdc_clk_div(gmacdev);

	/* search phy */


	gmacdev->PhyBase = 0;
	synopGMAC_check_phy_init(gmacdev);

#if 0
	phy_id = synopGMAC_search_phy(gmacdev);
	if (phy_id >= 0) {
		printf("====>found PHY %d\n", phy_id);
		gmacdev->PhyBase = phy_id;
	} else {
		printf("====>PHY not found!\n");
	}
#endif
	jz47xx_mac_configure();
	/* setup tx_desc */
	for (i = 0; i <  NUM_TX_DESCS; i++) {
		synopGMAC_tx_desc_init_ring(tx_desc + i, i == (NUM_TX_DESCS - 1));
	}

	synopGMACWriteReg((u32 *)gmacdev->DmaBase,DmaTxBaseAddr, virt_to_phys(_tx_desc));

	/* setup rx_desc */
	for (i = 0; i < NUM_RX_DESCS; i++) {
		DmaDesc *curr_desc = rx_desc + i;
		synopGMAC_rx_desc_init_ring(curr_desc, i == (NUM_RX_DESCS - 1));

		curr_desc->length |= ((PKTSIZE_ALIGN <<DescSize1Shift) & DescSize1Mask) |
			((0 << DescSize2Shift) & DescSize2Mask);
		curr_desc->buffer1 = virt_to_phys(NetRxPackets[i]);

		curr_desc->extstatus = 0;
		curr_desc->reserved1 = 0;
		curr_desc->timestamplow = 0;
		curr_desc->timestamphigh = 0;

		/* start transfer */
		curr_desc->status = DescOwnByDma;
	}

	synopGMACWriteReg((u32 *)gmacdev->DmaBase,DmaRxBaseAddr, virt_to_phys(_rx_desc));

	flush_dcache_all();

	//jz47xx_mac_configure();

#ifdef SYNOP_DEBUG
	jzmac_dump_all_regs(__func__, __LINE__);
#endif
	/* we only enable rx here */
	synopGMAC_enable_dma_rx(gmacdev);
//	synopGMAC_enable_dma_tx(gmacdev);

#if 1
	synopGMAC_tx_enable(gmacdev);
	synopGMAC_rx_enable(gmacdev);

#endif
	printf("GMAC init finish\n");
	return 1;
}

static void jz_halt(struct eth_device *dev)
{
	next_tx = 0;
	next_rx = 0;
	synopGMAC_rx_disable(gmacdev);
	udelay(100);
	synopGMAC_disable_dma_rx(gmacdev);
	udelay(100);
	synopGMAC_disable_dma_tx(gmacdev);
	udelay(100);
	synopGMAC_tx_enable(gmacdev);
}

int jz_net_initialize(bd_t *bis)
{
	struct eth_device *dev;
/*
	gmacdev = (synopGMACdevice *)malloc(sizeof(synopGMACdevice));
	if(gmacdev == NULL) {
		printf("synopGMACdevice malloc fail\n");
		return -1;
	}
*/
	gmacdev = &_gmacdev;
#define JZ_GMAC_BASE 0xb34b0000
	gmacdev->DmaBase =  JZ_GMAC_BASE + DMABASE;
	gmacdev->MacBase =  JZ_GMAC_BASE + MACBASE;
#ifndef CONFIG_FPGA
	/* reset DM9161 */
	gpio_direction_output(CONFIG_GPIO_DM9161_RESET, CONFIG_GPIO_DM9161_RESET_ENLEVEL);
	mdelay(10);
	gpio_set_value(CONFIG_GPIO_DM9161_RESET, !CONFIG_GPIO_DM9161_RESET_ENLEVEL);
	mdelay(10);

	/* initialize jz4775 gpio */
	gpio_set_func(GPIO_PORT_B, GPIO_FUNC_1, 0x0003fc10);
	gpio_set_func(GPIO_PORT_D, GPIO_FUNC_1, 0x3c000000);
	gpio_set_func(GPIO_PORT_F, GPIO_FUNC_0, 0x0000fff0);
	udelay(100000);
#else
	/* PB7 */
	gpio_set_func(GPIO_PORT_B, GPIO_FUNC_1, 0x00000080);
	udelay(10);

	/* reset PE10 */
	gpio_direction_output(CONFIG_GPIO_DM9161_RESET, CONFIG_GPIO_DM9161_RESET_ENLEVEL);
	udelay(10);
	gpio_direction_output(CONFIG_GPIO_DM9161_RESET, !CONFIG_GPIO_DM9161_RESET_ENLEVEL);
	udelay(10);

	/* output 1 PB7 */
	gpio_direction_output(32 * 1 + 7, CONFIG_GPIO_DM9161_RESET_ENLEVEL);
	udelay(100000);
#endif
	dev = (struct eth_device *)malloc(sizeof(struct eth_device));
	if(dev == NULL) {
		printf("struct eth_device malloc fail\n");
		return -1;
	}

	memset(dev, 0, sizeof(struct eth_device));

	sprintf(dev->name, "Jz4775-9161");

	dev->iobase	= 0;
	dev->priv	= 0;
	dev->init	= jz_init;
	dev->halt	= jz_halt;
	dev->send	= jz_send;
	dev->recv	= jz_recv;

	eth_register(dev);

	return 1;
}
