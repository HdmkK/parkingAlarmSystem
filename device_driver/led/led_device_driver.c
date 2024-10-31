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

#define DEV_NAME "led_device"

#define GREEN_LED 524 // pin 12
#define RED_LED 531 //pin 19


MODULE_LICENSE("GPL");


static int led_write(struct file *file, const char *buf, size_t len, loff_t *lof){
	int minor = iminor(file->f_inode);

	int value = *(int*)buf;

	if (value != 0 && value != 1) return -1;

	if (minor == 0){
		// control green led
		//printk(KERN_NOTICE "GREEN ON");
		gpio_set_value(GREEN_LED, value);

	}
	else if (minor == 1){
		// control red led
		//printk(KERN_NOTICE "RED ON");
		gpio_set_value(RED_LED, value);
	}
	else{
		return -1;
	}
	return 0;
}

struct file_operations led_fops = {
	.write = led_write
};

static dev_t dev_num;
static struct cdev *cd_cdev;

static int __init led_init(void){

	
	//printk("[led device] : INIT");

	gpio_request_one(GREEN_LED, GPIOF_OUT_INIT_LOW, "GREEN_LED");
	gpio_request_one(RED_LED, GPIOF_OUT_INIT_LOW, "RED_LED");
	
	alloc_chrdev_region(&dev_num, 0, 2, DEV_NAME);
	cd_cdev = cdev_alloc();
	cdev_init(cd_cdev, &led_fops);
	cdev_add(cd_cdev, dev_num, 2);

	return 0;
}

static void __exit led_exit(void){
	//printk("[led device] : EXIT");

	cdev_del(cd_cdev);
	unregister_chrdev_region(dev_num, 2);
	gpio_free(GREEN_LED);
	gpio_free(RED_LED);
}

module_init(led_init);
module_exit(led_exit);