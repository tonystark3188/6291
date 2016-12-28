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

#define WIFI_MODE_BTN			GPIO_PC(22)


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

#define GET_WIFI_MODE_BTN		0x30

#define SPEAKER_ON			0x41
#define SPEAKER_OFF			0x40

#define DUT_EN_ON			0x51
#define DUT_EN_OFF			0x50

#define USB_CONNECT			0x61
#define USB_DISCONNECT		0x60



static unsigned int cmd=0;

static int jz_gpio_init(void)
{

	gpio_request(WIFI_MODE_BTN, "wifi mode btn");
	gpio_direction_input(WIFI_MODE_BTN);
	
	return 0;
err:
	return -EINVAL;
}


static long jz_gpio_ioctl(struct file *file, unsigned int action,unsigned long arg)
{
	unsigned char btn_status = 0 ;
	cmd = action;
	
	
	//mod_timer(&timer_led_3g, jiffies + HZ*1000);
	//printk(" proc_encryp_ioctl=cmd=%x \n",cmd);
	switch (cmd) {
	

	case GET_WIFI_MODE_BTN:
		btn_status=__gpio_get_value(WIFI_MODE_BTN);
		if (copy_to_user((void __user *)arg,&btn_status, sizeof(btn_status)))
			return -EFAULT;
		
		break;


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





