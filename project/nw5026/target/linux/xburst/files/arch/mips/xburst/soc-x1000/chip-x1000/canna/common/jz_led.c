#include <linux/module.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/gpio.h>
#include <linux/proc_fs.h>
#include <linux/syscore_ops.h>
#include <linux/delay.h>
#include <irq.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/version.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/leds.h>
#include <linux/dma-mapping.h>

#include <gpio.h>
#include <soc/gpio.h>



// #include <linux/types.h>
// #include <linux/version.h>
// #include <linux/init.h>
// #include <linux/delay.h>
// #include <linux/err.h>
// #include <linux/platform_device.h>
// #include <asm/mach-ralink/rt_mmap.h>
// #include <linux/dma-mapping.h>
// #include <linux/leds.h>
// #include "ralink_gpio.h"


#define LED_BLUE 			GPIO_PC(26)  //blue led , 5G
#define WIFI_LED                GPIO_PC(25)  //green led , 2.4G

static struct gpio_led gpio_leds[] = {
        {
                .name   = "wifi:led",
                .gpio   = WIFI_LED,
                .active_low = 1,
                //.default_state = LEDS_GPIO_DEFSTATE_ON,
				.default_state = LEDS_GPIO_DEFSTATE_KEEP,
        },
/*
        {
                .name   = "longsys:green:led",
                .gpio   = LED_GREEN,
                .active_low = 1,
                //.default_state = LEDS_GPIO_DEFSTATE_OFF,
				.default_state = LEDS_GPIO_DEFSTATE_KEEP,
        },
*/
};

static struct gpio_led_platform_data gpio_led_info = {
        .leds           = gpio_leds,
        .num_leds       = ARRAY_SIZE(gpio_leds),
};

static struct platform_device leds_gpio = {
        .name   = "leds-gpio",
        .id     = -1,
        .dev    = {
                .platform_data  = &gpio_led_info,
        },
};


int __init led_init(void)
{
	// u32 gpiomode = le32_to_cpu(*(volatile u32 *)(RALINK_REG_GPIOMODE2));
	// gpiomode |= (1<<0);  //clear bit[0] WLAN_LED
	// *(volatile u32 *)(RALINK_REG_GPIOMODE2) = cpu_to_le32(gpiomode);
	platform_device_register(&leds_gpio);
	printk(">>>>>>>>>>register my wifi led<<<<<<<<<<<<<<<<<\r\n");
	return 0;
}

device_initcall(led_init);




