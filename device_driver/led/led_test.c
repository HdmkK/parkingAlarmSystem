#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#define GREEN_LED_DEVICE "/dev/led_device0"
#define RED_LED_DEVICE "/dev/led_device1"

int main(){

	int green_fd, red_fd;
	int value;

	green_fd = open(GREEN_LED_DEVICE, O_WRONLY);
	red_fd = open(RED_LED_DEVICE, O_WRONLY);

	

	for (int i = 0; i < 100; i++){

		value = i % 2;
		write(green_fd, &value, sizeof(int));
		sleep(1);
		write(red_fd, &value, sizeof(int));
		sleep(1);
	}

	close(green_fd);
	close(red_fd);

	return 0;
}