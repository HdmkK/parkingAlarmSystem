obj-m := led_device_driver.o

KERNELDIR := /rpi-kernel/linux/
PWD := $(shell pwd)

CROSS_COMPILE ?= arm-linux-gnueabihf-


default:
		$(MAKE) -C $(KERNELDIR) M=$(PWD) ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE) modules
		$(CROSS_COMPILE)gcc -o led_test led_test.c
clean:
		$(MAKE) -C $(KERNELDIR) M=$(PWD) ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE) clean
		rm led_test