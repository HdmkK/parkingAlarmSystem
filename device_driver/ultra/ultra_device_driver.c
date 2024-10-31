#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/delay.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/uaccess.h>
#include <linux/wait.h>

#define DEV_NAME "ultra_device"

#define TRIG_PIN 532 // pin 20
#define ECHO_PIN 533 //pin 21


MODULE_LICENSE("GPL");

static int irq_num;
static int ultra_state = 3;
static ktime_t echo_start;
static ktime_t echo_stop;
static int cm;

static DECLARE_WAIT_QUEUE_HEAD(wait_queue);

static irqreturn_t ultra_isr(int irq, void* dev_id){

	
	ktime_t tmp_time;
	u64 time_us;
	

	tmp_time = ktime_get();
	if (ultra_state == 1){
		//printk("[ultra device] : ECHO UP\n");

		if (gpio_get_value(ECHO_PIN) == 1){
			echo_start = tmp_time;
			ultra_state = 2;
		}
	}
	else if (ultra_state == 2){
		//printk("[ultra device] : ECHO DOWN\n");

		if (gpio_get_value(ECHO_PIN) == 0){

			echo_stop = tmp_time;
			time_us = ktime_to_us(ktime_sub(echo_stop, echo_start));
			cm = time_us;
			printk("[ultra device] : Detect %d cm\n", cm);
			ultra_state = 3;
			wake_up_interruptible(&wait_queue);
		}
	}

	return IRQ_HANDLED;
}


static int ultra_read(struct file *file, char*buf, size_t len, loff_t *lof){
	printk("[ultra device] : read\n");
	int ret;
	if (ultra_state == 3){
		ultra_state = 1;
		echo_start = ktime_set(0,0);
		echo_stop = ktime_set(0,0);

		gpio_set_value(TRIG_PIN, 1);
		udelay(10);
		gpio_set_value(TRIG_PIN, 0);

		ultra_state = 1;

		//need to time out, if time elapse, ultra_state = 3, and wake up using kernel timer forcefully!!

		wait_event_interruptible(wait_queue, ultra_state == 3);
		printk("[ultra device] : waked up\n");

		ret = copy_to_user(buf, &cm, sizeof(int));
		printk("\n\ncopy_to_user : %d\n", ret);
		printk("user copied : %d\n\n", *(int*)buf);
	}

	return 0;
}

struct file_operations ultra_fops = {
	.read = ultra_read
};

static dev_t dev_num;
static struct cdev *cd_cdev;

static int __init ultra_init(void){

	int ret;
	printk("[ultra device] : INIT");

	gpio_request_one(TRIG_PIN, GPIOF_OUT_INIT_LOW, "ULTRA_TRIG");
	gpio_request_one(ECHO_PIN, GPIOF_IN, "ULTRA_ECHO");


	irq_num = gpio_to_irq(ECHO_PIN);
	ret = request_irq(irq_num, ultra_isr, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "ULTRA_ECHO", NULL);

	alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
	cd_cdev = cdev_alloc();
	cdev_init(cd_cdev, &ultra_fops);
	cdev_add(cd_cdev, dev_num, 1);

	return 0;
}

static void __exit ultra_exit(void){
	printk("[ultra device] : EXIT");

	cdev_del(cd_cdev);
	unregister_chrdev_region(dev_num, 1);
	free_irq(irq_num, NULL);
}

module_init(ultra_init);
module_exit(ultra_exit);