#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#define DEV_NAME "/dev/ultra_device"

int main(){

	int fd;
	int cm;
	fd = open(DEV_NAME, O_RDONLY);

	//for (int i = 0; i < 5; i++){
	while(1){
		read(fd, &cm, sizeof(int));

		cm /= 58;
		printf("Detect %d\n", cm);
		sleep(1);
	}
}