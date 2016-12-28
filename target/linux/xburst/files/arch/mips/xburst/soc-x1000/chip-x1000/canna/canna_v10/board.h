#ifndef __BOARD_H__
#define __BOARD_H__
#include <gpio.h>
#include <soc/gpio.h>


#ifdef  CONFIG_BROADCOM_RFKILL
#define BLUETOOTH_UART_GPIO_PORT        GPIO_PORT_D
#define BLUETOOTH_UART_GPIO_FUNC        GPIO_FUNC_2
#define BLUETOOTH_UART_FUNC_SHIFT       0x4
#define HOST_WAKE_BT    GPIO_PG(6)
#define BT_WAKE_HOST    GPIO_PF(7)
#define BT_REG_EN       GPIO_PG(9)
#define BT_UART_RTS     GPIO_PD(29)
#define BT_PWR_EN   -1
#define HOST_BT_RST    -1
#define GPIO_PB_FLGREG      (0x10010158)
#define GPIO_BT_INT_BIT     (1 << (GPIO_BT_INT % 32))
#define BLUETOOTH_UPORT_NAME  "ttyS1"
#endif

/* MSC GPIO Definition */
#define GPIO_SD0_CD_N       GPIO_PC(21)

/*wifi*/
#define GPIO_WLAN_PW_EN     -1//GPIO_PD(24)
#define WL_WAKE_HOST        GPIO_PG(7)
#define WL_REG_EN       GPIO_PG(8)

#ifdef CONFIG_SPI_GPIO
#define GPIO_SPI_SCK  GPIO_PA(26)
#define GPIO_SPI_MOSI GPIO_PA(29)
#define GPIO_SPI_MISO GPIO_PA(28)
#endif

/* ****************************GPIO USB START******************************** */
#define GPIO_USB_ID             GPIO_PB(4) /*GPIO_PD(2)*/
#define GPIO_USB_ID_LEVEL       LOW_ENABLE
#define GPIO_USB_DETE           GPIO_PC(24) /*GPIO_PB(8)*/
#define GPIO_USB_DETE_LEVEL     LOW_ENABLE
#define GPIO_USB_DRVVBUS        GPIO_PB(25)
#define GPIO_USB_DRVVBUS_LEVEL      HIGH_ENABLE
/* ****************************GPIO USB END********************************** */

/* ****************************GPIO AUDIO START****************************** */
#define GPIO_HP_MUTE        -1  /*hp mute gpio*/
#define GPIO_HP_MUTE_LEVEL  -1  /*vaild level*/

#define GPIO_SPEAKER_EN    GPIO_PC(25)         /*speaker enable gpio*/
#define GPIO_SPEAKER_EN_LEVEL   0

#define GPIO_HANDSET_EN     -1  /*handset enable gpio*/
#define GPIO_HANDSET_EN_LEVEL   -1

#define GPIO_HP_DETECT  GPIO_PC(6)      /*hp detect gpio*/
#define GPIO_HP_INSERT_LEVEL    1
#define GPIO_MIC_SELECT     -1  /*mic select gpio*/
#define GPIO_BUILDIN_MIC_LEVEL  -1  /*builin mic select level*/
#define GPIO_MIC_DETECT     -1
#define GPIO_MIC_INSERT_LEVEL   -1
#define GPIO_MIC_DETECT_EN  -1  /*mic detect enable gpio*/
#define GPIO_MIC_DETECT_EN_LEVEL -1 /*mic detect enable gpio*/

#define HP_SENSE_ACTIVE_LEVEL   1
#define HOOK_ACTIVE_LEVEL       -1
/* ****************************GPIO AUDIO END******************************** */

#define GPIO_EFUSE_VDDQ			-ENODEV		/* EFUSE must be -ENODEV or a gpio */

#endif /* __BOARD_H__ */
