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


#define LED_SYS_YELLOW               GPIO_PC(27) 
#define LED_SYS_GREEN                GPIO_PC(26)  
#define LED_SYS_BLUE                 GPIO_PB(23)  

#define LED_WIFI_RED                 GPIO_PC(25) 
#define LED_WIFI_GREEN               GPIO_PD(5)
#define LED_WIFI_BLUE                GPIO_PD(4)


static struct gpio_led gpio_leds[] = {
        {
                .name   = "led:sys:yellow",
                .gpio   = LED_SYS_YELLOW,
                .active_low = 1,
                //.default_state = LEDS_GPIO_DEFSTATE_ON,
		.default_state = LEDS_GPIO_DEFSTATE_KEEP,
        },
        {
                .name   = "led:sys:green",
                .gpio   = LED_SYS_GREEN,
                .active_low = 1,
                //.default_state = LEDS_GPIO_DEFSTATE_ON,
                .default_state = LEDS_GPIO_DEFSTATE_KEEP,
        },
        {
                .name   = "led:sys:blue",
                .gpio   = LED_SYS_BLUE,
                .active_low = 1,
                //.default_state = LEDS_GPIO_DEFSTATE_ON,
                .default_state = LEDS_GPIO_DEFSTATE_KEEP,
        },
        {
                .name   = "led:wifi:red",
                .gpio   = LED_WIFI_RED,
                .active_low = 1,
                //.default_state = LEDS_GPIO_DEFSTATE_ON,
                .default_state = LEDS_GPIO_DEFSTATE_KEEP,
        },
        {
                .name   = "led:wifi:green",
                .gpio   = LED_WIFI_GREEN,
                .active_low = 1,
                //.default_state = LEDS_GPIO_DEFSTATE_ON,
                .default_state = LEDS_GPIO_DEFSTATE_KEEP,
        },
        {
                .name   = "led:wifi:blue",
                .gpio   = LED_WIFI_BLUE,
                .active_low = 1,
                //.default_state = LEDS_GPIO_DEFSTATE_ON,
                .default_state = LEDS_GPIO_DEFSTATE_KEEP,
        },
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




