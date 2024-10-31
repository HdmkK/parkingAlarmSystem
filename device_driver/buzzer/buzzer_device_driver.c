#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/gpio.h>


#define DEV_NAME "buzzer_device"

#define NUMBER_START 0x80
#define NUMBER1 NUMBER_START+1
#define NUMBER2 NUMBER_START+2

#define MAGIC_NUMBER 'z'
#define BUZZER_ON_CMD _IO(MAGIC_NUMBER, NUMBER1)
#define BUZZER_OFF_CMD _IO(MAGIC_NUMBER, NUMBER2)

//#define GPIO_BASE 0xFE200000
#define BUZZER_PIN 530 //pin 18

MODULE_LICENSE("GPL");


//buzzer on
static int buzzer_on(int pin){
	printk(KERN_NOTICE "[buzzer_device] : on!");
	gpio_set_value(BUZZER_PIN, 1);
	return 0;
}

//buzzer off
static int buzzer_off(int pin){
	printk(KERN_NOTICE "[buzzer_device] : off!");
	gpio_set_value(BUZZER_PIN, 0);
	return 0;
}

static long buzzer_ioctl(struct file *file, unsigned int cmd, unsigned long arg){

	printk("ioctl call\n");
	switch(cmd){
	case BUZZER_ON_CMD:
		buzzer_on(BUZZER_PIN);
		break;
	case BUZZER_OFF_CMD:
		buzzer_off(BUZZER_PIN);
		break;
	default:
		printk("unkown ioctl cmd\n");
		return -EINVAL;// I'll correct return error later..
	}

	return 0;
}

struct file_operations buzzer_fops = {
	.unlocked_ioctl = buzzer_ioctl
};

static dev_t dev_num;
static struct cdev *cd_cdev;

static int __init buzzer_init(void){

	int ret;

	printk(KERN_NOTICE "[buzzer_device] : INIT \n");

	gpio_request_one(BUZZER_PIN, GPIOF_OUT_INIT_LOW, "BUZZER");

	ret = alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);


	cd_cdev = cdev_alloc();
	cdev_init(cd_cdev, &buzzer_fops);
	ret = cdev_add(cd_cdev, dev_num, 1);

	return 0;
}

static void __exit buzzer_exit(void){
	printk(KERN_NOTICE "[buzzer_device] : RELEASE\n");
	cdev_del(cd_cdev);
	unregister_chrdev_region(dev_num, 1);
	gpio_free(BUZZER_PIN);
}

module_init(buzzer_init);
module_exit(buzzer_exit);