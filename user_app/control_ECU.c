#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include "buzzer_ioctl.h"


#define BUZZER "/dev/buzzer_device"
#define GREEN_LED "/dev/led_device0"
#define RED_LED "/dev/led_device1"

#define THRESHOLD 50

typedef struct queue_node{
	int data;
	struct queue_node *next;
}NODE;

pthread_mutex_t mutex;

void* update_thread(void* data);
void* control_thread(void* data);
void* receive_thread(void* data);

NODE* create_node(int data);
void enqueue(NODE* new_node);
NODE* dequeue();
void print_queue();


sem_t consume;

int buzzer_fd, green_led_fd, red_led_fd;
int cur_distance_data;
int cur_led_status; // 0 : green, 1 : red

int main(){

	pthread_t pthread[4];
	int status;

	sem_init(&consume, 0, 0);
	pthread_mutex_init(&mutex, NULL);


	buzzer_fd = open(BUZZER, O_RDONLY);
	green_led_fd = open(GREEN_LED, O_WRONLY);
	red_led_fd = open(RED_LED, O_WRONLY);

	char p1[] = "update_thread";
	char p2[] = "send_thread";
	char p3[] = "receive_thread";
	char p4[] = "control_thread";

	//create update thread
	if (pthread_create(&pthread[0], NULL, update_thread, (void*)p1) < 0){
		perror("[control_ECU] : fail to create update_thread");
		exit(EXIT_FAILURE);
	}

	//create receive thread
	if (pthread_create(&pthread[2], NULL, receive_thread, (void*)p2) < 0){
		perror("[control_ECU] : fail to create receive_thread");
		exit(EXIT_FAILURE);
	}

	//complete

	//create control thread
	if (pthread_create(&pthread[3], NULL, control_thread, (void*)p3) < 0){
		perror("[control_ECU] : fail to create control_thread");
		exit(EXIT_FAILURE);
	}
	

	//pthread_join(pthread[0], (void**)&status);
	pthread_join(pthread[2], (void**)&status);
	//pthread_join(pthread[3], (void**)&status);


	pthread_mutex_destroy(&mutex);
	return 0;
}

void* update_thread(void* data){
	

	NODE* new_node = NULL;
	



	while (1){

		sem_wait(&consume);

		pthread_mutex_lock(&mutex);
		new_node = dequeue();		
		pthread_mutex_unlock(&mutex);
	
		printf("%d\n", new_node->data);
		
		cur_distance_data = (new_node->data)/58;

		free(new_node);
		new_node = NULL;

	}

}


void* control_thread(void* data){

	//see current distance data and adjust buzzer&led;

	int led_value;
	int buzzer_interval;

	while (1){

		//renew led
		//printf("cur_distance_data : %d\n", cur_distance_data);

		if ( cur_distance_data > THRESHOLD){
			led_value = 0;
			write(red_led_fd, &led_value, sizeof(int));
			led_value = 1;
			write(green_led_fd, &led_value, sizeof(int));
		}

		if ( cur_distance_data <= THRESHOLD){
			led_value = 0;
			write(green_led_fd, &led_value, sizeof(int));
			led_value = 1;
			write(red_led_fd, &led_value, sizeof(int));
		}



		//adjust buzzer


		if ( cur_distance_data > THRESHOLD){
			ioctl(buzzer_fd, BUZZER_OFF_CMD, NULL);
			continue;
		}

		buzzer_interval = cur_distance_data * 10000;
		//printf("%d\n", buzzer_interval);
		ioctl(buzzer_fd, BUZZER_ON_CMD, NULL);
		//sleep(1);
		usleep(buzzer_interval);
		ioctl(buzzer_fd, BUZZER_OFF_CMD, NULL);
		//sleep(1);
		usleep(buzzer_interval);

		//usleep(buzzer_interval);

	}


}


#define PORT 8888

void* receive_thread(void* data){
	//receive distance data from sensing ECU periodically and add data into queue

	int distance_data;
	int fd;


	int serv_sock_fd, clnt_sock_fd;
	struct sockaddr_in serv_addr, clnt_addr;
	socklen_t clnt_addr_size;

	serv_sock_fd = socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(PORT);

	if (bind(serv_sock_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		perror("bind() error");

	if (listen(serv_sock_fd, 5) == -1)
		perror("listen() error");

	clnt_addr_size = sizeof(clnt_addr);
	clnt_sock_fd = accept(serv_sock_fd, (struct sockaddr*)&clnt_addr, &clnt_addr_size);

	//int test_data[] = {50, 45, 40, 35, 30, 25, 20, 15, 10};

	NODE* new_node = NULL;

	printf("start receive\n");

	while(1){
	//for (int i = 0; i < sizeof(test_data)/sizeof(test_data[0]); i++){
		
		read(clnt_sock_fd, &distance_data, sizeof(int));
		//distance_data = test_data[i];

		//printf("[contol_ECU] : received data %d\n", distance_data);
		new_node = create_node(distance_data);
		
		pthread_mutex_lock(&mutex);
		enqueue(new_node);
		pthread_mutex_unlock(&mutex);

		sem_post(&consume);
	}

	close(serv_sock_fd);
	close(clnt_sock_fd);

}


//----queue----//


NODE *head, *tail;


NODE* create_node(int data){

	NODE* new_node = (NODE*)malloc(sizeof(NODE));

	if (new_node == NULL) return NULL;

	new_node->data = data;
	new_node->next = NULL;

	return new_node;
}

void enqueue(NODE* new_node){

	if (head == NULL) {
		head = tail = new_node;
		return;
	}

	tail->next = new_node;
	tail = new_node;

}

NODE* dequeue(){

	NODE* target_node = NULL;

	if (head == NULL) return NULL;

	target_node = head;
	target_node->next = NULL;
	head = head->next;

	return target_node;

}

void print_queue(){
	printf("[PRINT] : ");
	for (NODE* cur = head; cur != NULL; cur = cur->next){
		printf("%d, ", cur->data);
	}
}



