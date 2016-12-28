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

#include <gpio.h>
#include <soc/gpio.h>

//#define LED_D3		GPIO_PB(1)   /* evb board */
#define LED_D3 			GPIO_PC(25)

#define START_BTN			23
#define WIFI_LED  			0
#define RLED				26
#define GLED				27
#define USB_5V_EN			8
#define HUB_EN				6
#define SPEAKER				7
#define DUT_EN				21

// #define LED_ON				1
// #define LED_OFF				0
// #define LED_BLINK			2

#define LED_ON 				0x11
#define LED_OFF				0x10

#define RLED_ON				0x11
#define RLED_OFF			0x10
#define RLED_BLINK			0x12

#define GLED_ON				0x21
#define GLED_OFF			0x20
#define GLED_BLINK			0x22

#define GET_START_BTN		0x30

#define SPEAKER_ON			0x41
#define SPEAKER_OFF			0x40

#define DUT_EN_ON			0x51
#define DUT_EN_OFF			0x50



static unsigned int cmd=0;

static int jz_gpio_init(void)
{
	int led=LED_D3;
	printk(">>>>>>>>>> jz_gpio_ctl init <<<<<<<<<<<\n");
	if (gpio_request(led, "led d3")) {
		pr_err("ERROR: no led d3 pin available !!\n");
		goto err;
	} else {
		gpio_direction_output(led, 0);
	}

	gpio_set_value(led, 0);

	return 0;
err:
	return -EINVAL;
}


static long jz_gpio_ioctl(struct file *file, unsigned int action,unsigned long arg)
{
	cmd = action;
	
	
	//mod_timer(&timer_led_3g, jiffies + HZ*1000);
	//printk(" proc_encryp_ioctl=cmd=%x \n",cmd);
	switch (cmd) {
	case LED_ON:
		gpio_set_value(LED_D3, 0);
		
		break;
	case LED_OFF:
		gpio_set_value(LED_D3, 1);
		
		break;
#if 0
	case RLED_BLINK:
		mod_timer(&timer_red_led, jiffies + HZ/2);
		break;

	case GET_START_BTN:
		btn_status=__gpio_get_value(START_BTN);
		if (copy_to_user((void __user *)arg,&btn_status, sizeof(btn_status)))
			return -EFAULT;
		
		break;
#endif	
	default:
		
		printk("Not supported action!\n");
		break;	
	}
	return 0;
}
static int jz_gpio_open(struct inode *inode, struct file *file)
{
//	printk("proc_encryp_open\n\n");
	return 0;
}
#if 0
static int jz_gpio_read(struct inode *inode, struct file *file)
{
//	printk("proc_encryp_open\n\n");
	return 0;
}
static int jz_gpio_write(struct inode *inode, struct file *file)
{
//	printk("proc_encryp_open\n\n");
	return 0;
}
#endif
static struct file_operations gpio_ctl_fops = { 
open:jz_gpio_open,
read:NULL, 
write:NULL,
unlocked_ioctl:jz_gpio_ioctl, 

}; 


static int __init jz_gpio_init_proc(void)
{

	struct proc_dir_entry *res=NULL;
	jz_gpio_init();

	res = create_proc_entry("jz_gpio", 0, NULL);
	if (res) {
		
		res->proc_fops = &gpio_ctl_fops;
	}

	return 0;
}

module_init(jz_gpio_init_proc);

























#if 0

#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/reboot.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/list.h>
#include <linux/proc_fs.h>
#include <linux/wifi_led_control.h>
//#define GPIO_27		23

#define START_BTN			23
#define WIFI_LED  			0
#define RLED				26
#define GLED				27
#define USB_5V_EN			8
#define HUB_EN				6
#define SPEAKER				7
#define DUT_EN				21

// #define LED_ON				1
// #define LED_OFF				0
// #define LED_BLINK			2

#define RLED_ON				0x11
#define RLED_OFF			0x10
#define RLED_BLINK			0x12

#define GLED_ON				0x21
#define GLED_OFF			0x20
#define GLED_BLINK			0x22

#define GET_START_BTN		0x30

#define SPEAKER_ON			0x41
#define SPEAKER_OFF			0x40

#define DUT_EN_ON			0x51
#define DUT_EN_OFF			0x50


static struct timer_list timer_red_led,timer_green_led;
static unsigned int cmd=0;

static void red_led_blink(){

	if(cmd==RLED_OFF)
	{
		//del_timer(&timer_led_3g);
		__gpio_set_value(RLED,0);
	}
	else if(cmd==RLED_ON)
	{
		//del_timer(&timer_led_3g);
		__gpio_set_value(RLED,1);
	}
	else if(cmd==RLED_BLINK)
	{
		if(__gpio_get_value(RLED))
			__gpio_set_value(RLED,0);
		else
			__gpio_set_value(RLED,1);
		mod_timer(&timer_red_led, jiffies + HZ/2);
//		printk("timer start...\n");
//		mod_timer(&timer_led_3g, jiffies + 2*HZ);
	}
	else
	{
		printk("Not supported action!\n");
	}
}

static void green_led_blink(){

	if(cmd==GLED_OFF)
	{
		//del_timer(&timer_led_3g);
		__gpio_set_value(GLED,0);
	}
	else if(cmd==GLED_ON)
	{
		//del_timer(&timer_led_3g);
		__gpio_set_value(GLED,1);
	}
	else if(cmd==GLED_BLINK)
	{
		if(__gpio_get_value(GLED))
			__gpio_set_value(GLED,0);
		else
			__gpio_set_value(GLED,1);
		mod_timer(&timer_green_led, jiffies + HZ/2);
//		printk("timer start...\n");
//		mod_timer(&timer_led_3g, jiffies + 2*HZ);
	}
	else
	{
		printk("Not supported action!\n");
	}
}

static long proc_3g_ioctl(struct file *file, unsigned int action,unsigned long arg)
{
	cmd = action;
	unsigned char btn_status;
	// mod_timer(&timer_led_3g, jiffies + HZ*1000);
//	printk(" proc_encryp_ioctl=cmd=%x \n",cmd);
	switch (cmd) {
	case RLED_ON:
		__gpio_set_value(RLED, 1);
		
		break;
	case RLED_OFF:
		__gpio_set_value(RLED, 0);
		
		break;
	case RLED_BLINK:
		mod_timer(&timer_red_led, jiffies + HZ/2);
		break;

	case GLED_ON:
		__gpio_set_value(GLED, 1);
		
		break;
	case GLED_OFF:
		__gpio_set_value(GLED, 0);

		break;
	case GLED_BLINK:
		mod_timer(&timer_green_led, jiffies + HZ/2);
		break;
	case GET_START_BTN:
		btn_status=__gpio_get_value(START_BTN);
		if (copy_to_user((void __user *)arg,&btn_status, sizeof(btn_status)))
			return -EFAULT;
		
		break;
	case SPEAKER_ON:
		__gpio_set_value(SPEAKER, 1);
		break;
	case SPEAKER_OFF:
		__gpio_set_value(SPEAKER, 0);
		break;
	case DUT_EN_ON:
		__gpio_set_value(DUT_EN, 1);
		break;
	case DUT_EN_OFF:
		__gpio_set_value(DUT_EN, 0);
		break;
	
	default:
		
		printk("Not supported action!\n");
		break;	
	}
	return 0;
}
static int proc_3g_open(struct inode *inode, struct file *file)
{
//	printk("proc_encryp_open\n\n");
	return 0;
}

static int proc_3g_read(struct inode *inode, struct file *file)
{
//	printk("proc_encryp_open\n\n");
	return 0;
}
static int proc_3g_write(struct inode *inode, struct file *file)
{
//	printk("proc_encryp_open\n\n");
	return 0;
}
static struct file_operations led_3g_fops = { 
open:proc_3g_open,
read: proc_3g_read, 
write: proc_3g_write,
unlocked_ioctl: proc_3g_ioctl, 

}; 

// #define START_BTN			23
// #define WIFI_LED  			0
// #define RLED				26
// #define GLED				27
// #define USB_5V_EN			8
// #define HUB_EN				6
// #define SPEAKER				7

static int __init led_3g_init(void)
{
	struct proc_dir_entry *res=NULL;

	gpio_request(START_BTN, "START_BTN");
	gpio_request(RLED, "RLED");
	gpio_request(GLED, "GLED");
	gpio_request(USB_5V_EN, "USB_5V_EN");
	gpio_request(HUB_EN, "HUB_EN");
	gpio_request(SPEAKER, "SPEAKER");
	gpio_request(DUT_EN, "DUT_EN");

	gpio_direction_input(START_BTN);
	gpio_direction_output(RLED, 1);
	gpio_direction_output(GLED, 1);
	gpio_direction_output(USB_5V_EN, 1);
	gpio_direction_output(HUB_EN, 1);
	gpio_direction_output(SPEAKER, 1);
	gpio_direction_output(DUT_EN, 1);

	// __gpio_set_value(START_BTN, 0);
	__gpio_set_value(RLED, 0);
	__gpio_set_value(GLED, 0);
	__gpio_set_value(USB_5V_EN, 1);
	__gpio_set_value(HUB_EN, 1);
	__gpio_set_value(SPEAKER, 0);
	__gpio_set_value(DUT_EN, 1);

	init_timer(&timer_red_led);
	timer_red_led.expires = jiffies + HZ/2;
	timer_red_led.data = (unsigned long)NULL;
	timer_red_led.function = &red_led_blink;
	add_timer(&timer_red_led);

	init_timer(&timer_green_led);
	timer_green_led.expires = jiffies + HZ/2;
	timer_green_led.data = (unsigned long)NULL;
	timer_green_led.function = &green_led_blink;
	add_timer(&timer_green_led);


//	gpio_direction_output(GPIO_27, 1);
	

	res = create_proc_entry("update_io_ctl", 0, NULL);
	if (res) {
		
		res->proc_fops = &led_3g_fops;
	}

	return 0;
}


static void __exit led_3g_exit(void)
{
	printk("Unload LED 3G GPIO Driver \n");
	// del_timer(&timer_led_3g);

	return 0;

}

module_exit(led_3g_exit);
module_init(led_3g_init);

MODULE_AUTHOR("Raphael Assenat <raph@8d.com>, Trent Piepho <tpiepho@freescale.com>");
MODULE_DESCRIPTION("LED 3G driver");
MODULE_LICENSE("GPL");


#endif
