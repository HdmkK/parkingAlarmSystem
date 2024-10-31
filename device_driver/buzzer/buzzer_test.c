#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "buzzer_ioctl.h"

int main(){
	int dev;
	int ret;
	dev = open("/dev/buzzer_device", O_RDONLY);
	printf("dev : %d\n", dev);

for (int i = 0; i < 5; i++){
	ret = ioctl(dev, BUZZER_ON_CMD, NULL);
	printf("ret : %d\n", ret);
	sleep(1);
	ret = ioctl(dev, BUZZER_OFF_CMD, NULL);
	printf("ret : %d\n", ret);
	sleep(1);
}

	
	close(dev);
}