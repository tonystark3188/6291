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

#include <gpio.h>
#include <soc/gpio.h>

//#define LED_D3		GPIO_PB(1)   /* evb board */
#define LED_D3 			GPIO_PC(25)
#define SYS_BTN			GPIO_PC(23)

extern unsigned long volatile jiffies;

#define HOLD_CLICK_DELAY CONFIG_HZ*3  //3s

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

#define GET_SYS_BTN			0x30

#define SPEAKER_ON			0x41
#define SPEAKER_OFF			0x40

#define DUT_EN_ON			0x51
#define DUT_EN_OFF			0x50

#define SET_RESET_ITP		0x60

typedef struct {
	unsigned int irq;		//request irq pin number
	pid_t pid;			//process id to notify
} jz_gpio_reg_info;

jz_gpio_reg_info jz_gpio_info;

void jz_gpio_notify_user(int usr)
{
	struct task_struct *p = NULL;

	//don't send any signal if pid is 0 or 1
	if ((int)jz_gpio_info.pid < 2)
		return;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
	p = find_task_by_vpid(jz_gpio_info.pid);
#else
	p = find_task_by_pid(jz_gpio_info.pid);
#endif

	if (NULL == p) {
		printk("no registered process to notify\n");
		return;
	}

	if (usr == 1) {
		printk("sending a SIGUSR1 to process %d\n",
				jz_gpio_info.pid);
		send_sig(SIGUSR1, p, 0);
	}
	else if (usr == 2) {
		printk("sending a SIGUSR2 to process %d\n",
				jz_gpio_info.pid);
		send_sig(SIGUSR2, p, 0);
	}
}
#if 0
static irqreturn_t jz_gpio_irq_handler(int irq, void *dev_id)
{
	int btn_status = 0 ;
	struct gpio_time_record {
		unsigned long falling;
		unsigned long rising;
	};
	static struct gpio_time_record record_reset;
	unsigned long now;
	
	now = jiffies;

	btn_status=__gpio_get_value(SYS_BTN);

	if (btn_status == 1) { //rising edge
		printk("rising edge\n");
		if (record_reset.rising != 0 && time_before_eq(now,
					record_reset.rising + 40L)) {
			/*
			 * If the interrupt comes in a short period,
			 * it might be floating. We ignore it.
			 */
			 printk("ignore\n");
		}
		else {
			record_reset.rising = now;
			if (time_before(now, record_reset.falling + HOLD_CLICK_DELAY)) {
				//one click
				printk("short click\n");
				jz_gpio_notify_user(1);
			}
			else {
				//press for several seconds
				printk("hold click\n");
				jz_gpio_notify_user(2);
			}
		}
	}
	else { //falling edge
		printk("falling edge\n");
		record_reset.falling = now;
	}
	
	printk("jz_gpio_irq_handler\n");
	return 0;
}
#endif

static irqreturn_t jz_gpio_irq_handler(int irq, void *dev_id)
{
	int btn_status = 0 ;
	struct gpio_time_record {
		unsigned long falling;
		unsigned long rising;
	};
	static struct gpio_time_record record_reset;
	unsigned long now;
	
	now = jiffies;

	btn_status=__gpio_get_value(SYS_BTN);
	//printk("jz_gpio_irq_handler\n");
	if (btn_status == 1) { //rising edge
		mdelay(10);
		btn_status = __gpio_get_value(SYS_BTN);
		if(btn_status != 1){
			//printk("rising ignore\n");
			goto EXIST;
		}		
		//printk("rising edge\n");
		record_reset.rising = now;
		jz_gpio_notify_user(2);
	}
	else { //falling edge
		mdelay(10);
		btn_status = __gpio_get_value(SYS_BTN);
		if(btn_status != 0){
			//printk("falling ignore\n");
			goto EXIST;
		}		
		//printk("falling edge\n");
		record_reset.falling = now;
		jz_gpio_notify_user(1);
	}
EXIST:
	return 0;
}


static int jz_gpio_init(void)
{
	printk(">>>>>>>>>> jz_gpio_ctl init <<<<<<<<<<<\n");
	
	return 0;
}

static long jz_gpio_ioctl(struct file *file, unsigned int action,unsigned long arg)
{
	int btn_status = 0 ;
	unsigned int cmd=0;
	jz_gpio_reg_info info;
	cmd = action;
		
	switch (cmd) {
		case LED_ON:
			if (gpio_request(LED_D3, "led d3")) {
				pr_err("ERROR: no led d3 pin available !!\n");
				goto err;
			} else {
				gpio_direction_output(LED_D3, 0);
			}
			
			gpio_set_value(LED_D3, 0);
			
			break;
		case LED_OFF:
			if (gpio_request(LED_D3, "led d3")) {
				pr_err("ERROR: no led d3 pin available !!\n");
				goto err;
			} else {
				gpio_direction_output(LED_D3, 0);
			}
			
			gpio_set_value(LED_D3, 1);
			
			break;

		case GET_SYS_BTN:
			if (gpio_request(SYS_BTN, "key reset")) {
				pr_err("ERROR: no key reset pin available !!\n");
				goto err;
			}

			if(gpio_direction_input(SYS_BTN)){
				pr_err("ERROR: set gpio input error !!\n");
				goto err;
			}
			
			btn_status=__gpio_get_value(SYS_BTN);
			printk("btn_status = %d\n", btn_status);
			if (copy_to_user((void __user *)arg,&btn_status, sizeof(btn_status)))
				return -EFAULT;
			break;

		case SET_RESET_ITP:		
			copy_from_user(&info, (jz_gpio_reg_info *)arg, sizeof(info));
			jz_gpio_info.pid = info.pid;
			jz_gpio_info.irq = info.irq;
			printk("jz_gpio_info.pid = %d, jz_gpio_info.irq = %d", jz_gpio_info.pid, jz_gpio_info.irq);

			if (gpio_request(info.irq, "key reset")) {
				pr_err("ERROR: no key reset pin available !!\n");
				goto err;
			}

			if(gpio_direction_input(info.irq)){
				pr_err("ERROR: set gpio input error !!\n");
				goto err;
			}
			
			if(request_irq(gpio_to_irq(info.irq), jz_gpio_irq_handler, IRQF_DISABLED, "jz_gpio_reset", NULL)){
				pr_err("ERROR: request_irq error !!\n");
				goto err;
			}

			if(irq_set_irq_type(gpio_to_irq(info.irq), IRQ_TYPE_EDGE_BOTH)){
				pr_err("ERROR: irq_set_irq_type error !!\n");
				goto err;
			}
			break;
		default:
			
			printk("Not supported action!\n");
			break;	
	}
	return 0;
err:
	return -EINVAL;
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
