#include <linux/mmc/host.h>
#include <linux/fs.h>
#include <linux/wlan_plat.h>
#include <asm/uaccess.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/gpio.h>
#include <linux/wakelock.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <mach/jzmmc.h>
#include <linux/bcm_pm_core.h>

#include <gpio.h>
#include <soc/gpio.h>

#define RESET               		0
#define NORMAL              		1

extern  void bcm_wlan_power_off(int);
extern  void bcm_wlan_power_on(int);

int mrvl_wlan_power_on(void)
{
		// jzmmc_manual_detect(1, 1);
		bcm_wlan_power_on(RESET);
		bcm_wlan_power_on(NORMAL);
		return 0;
}
EXPORT_SYMBOL(mrvl_wlan_power_on);
