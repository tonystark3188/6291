#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/reboot.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/list.h>
#include <linux/proc_fs.h>

#define KEY_SCAN 1
#define SHUTDOWN 2

#define PENKEYUP (GPIO_PB(8))
#define PENKEYDOWN (GPIO_PB(10))

#define POWERIO_EN (GPIO_PA(0))//power key io EN
#define PENKEYPOWER (GPIO_PA(2))//power key io detec
#define BATTERY_IO (GPIO_PC(23))


#define PENKEYUP_OFFSET (0)//01b
#define PENKEYDOWN_OFFSET (1)//10b
#define BATTERY_OFFSET (7)
#define PENKEYPOWERDOWN (0xf)
//#define PENKEYRECORD (3)//11b

static unsigned int cmd=0;


static int power_down()
{
    __gpio_set_value(POWERIO_EN, 0);
	udelay(5000);
	__gpio_set_value(POWERIO_EN, 0);
	return 0;
}

static int key_scan(void)
{
    int LedValue = 0;
     	
    if(0 == __gpio_get_value(PENKEYUP))//PAGEUP
    {   
        udelay(5000);
        if(__gpio_get_value(PENKEYUP) == 0)
            LedValue = LedValue | 1 << PENKEYUP_OFFSET;
    }
    if(0 ==__gpio_get_value(PENKEYDOWN))//PAGEDOWN
    {   
        udelay(5000);
        if(__gpio_get_value(PENKEYDOWN) == 0)
             LedValue = LedValue | 1 << PENKEYDOWN_OFFSET;
    }
    if(0 == __gpio_get_value(BATTERY_IO))//BATTERY CHECK IO
    {   
        udelay(5000);
        if(__gpio_get_value(BATTERY_IO) == 0)
             LedValue = LedValue | 1 << BATTERY_OFFSET;
    }
    
    
    if(0 == __gpio_get_value(PENKEYPOWER))
    {   
        udelay(5000);
        if(__gpio_get_value(PENKEYPOWER) == 0)
             LedValue = PENKEYPOWERDOWN;
    }
    return LedValue;
         
/****************test for canna base board********************
    gpio_direction_output(PENKEYDOWN,0);
    	printk("===%d\n",__gpio_get_value(PENKEYUP));

    	if(__gpio_get_value(PENKEYUP))
    	{   
         udelay(5000);
    		if(__gpio_get_value(PENKEYUP) == 1)
    		    return PENKEYUPVALUE;     		    
    	}
     return 0;
*/
}
static int proc_penkey_ioctl(struct file *file, unsigned int action)
{
    cmd = action;
	switch (cmd) {
	case KEY_SCAN:
		return key_scan();
		break;
	case SHUTDOWN:
        return power_down();
		break;
	default:
		break;	
	}
	return 0;
}
static int proc_penkey_open(struct inode *inode, struct file *file)
{
//	printk("proc_encryp_open\n\n");
	return 0;
}

static int proc_penkey_read(struct inode *inode, struct file *file)
{
//	printk("proc_encryp_open\n\n");
	return 0;
}
static int proc_penkey_write(struct inode *inode, struct file *file)
{
//	printk("proc_encryp_open\n\n");
	return 0;
}
static struct file_operations penkey_fops = { 
open:proc_penkey_open,
read: proc_penkey_read, 
write: proc_penkey_write,
unlocked_ioctl: proc_penkey_ioctl, 

}; 


static int __init penkey_init(void)
{
	struct proc_dir_entry *res=NULL;
	int ret=0;
	
	printk("JJJHHH ############# penkey_ioctl request io start\n");
     ret = gpio_request(PENKEYUP, "PENKEYUP");
	if (ret) {
		printk( "failed to request PENKEYUP IO\n");
		//return ret;
	}
	//else
		gpio_direction_input(PENKEYUP);
	ret = gpio_request(PENKEYDOWN, "PENKEYDOWN");
	if (ret) {
		printk("failed to request PENKEYDOWN IO\n");
		//return ret;
	}
	//else
		gpio_direction_input(PENKEYDOWN);
	ret = gpio_request(PENKEYPOWER, "PENKEYPOWER");
	if (ret) {
		printk( "failed to request PENKEYPOWER IO\n");
		//return ret;
	}
	//else
		gpio_direction_input(PENKEYPOWER);
	ret = gpio_request(BATTERY_IO, "BATTERY_IO");
	if (ret) {
		printk( "failed to request BATTERY_IO\n");
		//return ret;
	}
	//else
		gpio_direction_input(BATTERY_IO);
		
	ret = gpio_request(POWERIO_EN, "POWERIO_EN");
	if (ret) {
		printk( "failed to request POWERIO_EN IO\n");
		//return ret;
	}
	gpio_direction_output(POWERIO_EN,1);
//	gpio_direction_output(POWERIO_EN)
	__gpio_set_value(POWERIO_EN, 1);
	//else
		gpio_direction_output(POWERIO_EN,1);//set GPIO_PA(0) to 1-----power on

   printk("JJJHHH #############penkey_ioctl  request io end\n");

	res = create_proc_entry("penkey_ioctl", 0, NULL);
	if (res) {
		
		res->proc_fops = &penkey_fops;
	}
      
	return 0;
}


static void __exit penkey_exit(void)
{
	printk("Unload penkey GPIO Driver \n");

	gpio_free(PENKEYUP);
	gpio_free(PENKEYDOWN);
	gpio_free(POWERIO_EN);
	gpio_free(PENKEYPOWER);
	gpio_free(BATTERY_IO);

	return 0;
}

module_exit(penkey_exit);
module_init(penkey_init);

MODULE_AUTHOR("hu.jiang <hu.jiang@longsys.com>");
MODULE_DESCRIPTION("pen key");
MODULE_LICENSE("GPL");