obj-m := button_device_driver.o

KERNELDIR := /rpi-kernel/linux/
PWD := $(shell pwd)

CROSS_COMPILE ?= arm-linux-gnueabihf-


default:
		$(MAKE) -C $(KERNELDIR) M=$(PWD) ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE) modules
clean:
		$(MAKE) -C $(KERNELDIR) M=$(PWD) ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE) clean