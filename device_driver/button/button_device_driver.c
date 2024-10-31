#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/timer.h>

#define DEV_NAME "button_device"

#define BUTTON_PIN 529


MODULE_LICENSE("GPL");


static int button_irq;


struct button_timer_info{
	struct timer_list timer;
	long delay_jiffies;
	int irq_num;
};

static struct button_timer_info button_timer;

static void button_timer_func(struct timer_list* t){
	struct button_timer_info *info = from_timer(info, t, timer);

	enable_irq(info->irq_num);
}


//button interrupt service routine
static irqreturn_t button_isr(int irq, void* dev_id){

	printk(KERN_INFO "Button pressed!, irq num : %d\n", irq);

	disable_irq_nosync(button_irq);
	button_timer.delay_jiffies = msecs_to_jiffies(200);
	button_timer.irq_num = button_irq;
	timer_setup(&button_timer.timer, button_timer_func, 0);
	button_timer.timer.expires = jiffies + button_timer.delay_jiffies;
	add_timer(&button_timer.timer);

	return IRQ_HANDLED;
}




static int __init button_init(void){

	int ret;
	printk("[button device] : INIT");

	ret = gpio_request_one(BUTTON_PIN, GPIOF_IN, "SWITCH");
	if (ret) {
		printk(KERN_ALERT "[button device] : Failed to request Button GPIO\n");
		gpio_free(BUTTON_PIN);
		return ret;
	}


	gpio_direction_input(BUTTON_PIN);


	button_irq = gpio_to_irq(BUTTON_PIN);
	ret = request_irq(button_irq, button_isr, IRQF_TRIGGER_RISING, "button_irq", NULL);
	if (ret){
		printk("[button device] : Unable to reset request IRQ : %d\n", button_irq);
		free_irq(button_irq, NULL);
		gpio_free(BUTTON_PIN);
	}else{
		printk("[button device] : Enable to set request IRQ : %d\n", button_irq);
	}

	

	return 0;

}

static void __exit button_exit(void){
	printk("[button device] : EXIT");
	free_irq(button_irq, NULL);
	gpio_free(BUTTON_PIN);


	del_timer(&button_timer.timer);
}

module_init(button_init);
module_exit(button_exit);