#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

#define ULTRA_DEVICE "/dev/ultra_device"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8888

typedef struct queue_node{
	int data;
	struct queue_node *next;
}NODE;


void* ultra_thread(void* data);
void* send_thread(void* data);

NODE* create_node(int data);
void enqueue(NODE* new_node);
NODE* dequeue();
void print_queue();

pthread_mutex_t mutex;
sem_t consume;




void* ultra_thread(void* data){

	int value;
	int ret;
	int cm;

	NODE* new_node = NULL;

	int ultra_fd;
	ultra_fd = open(ULTRA_DEVICE, O_RDONLY);

	while (1){

		//printf("before read ultra\n");
		ret = read(ultra_fd, &cm, sizeof(int));
		//printf("result : %d\n", cm);

		new_node = create_node(cm);

		enqueue(new_node);
		
		sem_post(&consume);
		sleep(1);		
	}


	close(ultra_fd);
}

void* send_thread(void* data){

	int sock = socket(PF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
	serv_addr.sin_port = htons(SERVER_PORT);

	if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1){
		perror("connect() error!");	
	}

	printf("success connect!\n");


	int value;
	int ret;
	int cm;
	NODE* new_node;

	while (1){

		sem_wait(&consume);
		new_node = dequeue();
		printf("read from ultra : %d\n", new_node->data);
		value = new_node->data;
		write(sock, &value, sizeof(int));
		printf("send : %d\n", value);
		
	}

	close(sock);
}

int main(){

	pthread_t pthread[4];
	int status;

	sem_init(&consume, 0, 0);

	char p1[] = "ultra_thread";
	char p2[] = "send_thread";

	//create update thread
	if (pthread_create(&pthread[0], NULL, ultra_thread, (void*)p1) < 0){
		perror("[control_ECU] : fail to create ultra_thread");
		exit(EXIT_FAILURE);
	}

	//create receive thread
	if (pthread_create(&pthread[1], NULL, send_thread, (void*)p2) < 0){
		perror("[control_ECU] : fail to create _thread");
		exit(EXIT_FAILURE);
	}

	pthread_join(pthread[0], (void**)&status);
	pthread_join(pthread[1], (void**)&status);
	return 0;
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
