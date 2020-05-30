//share memory
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>

#include<sys/ipc.h>
#include<sys/sem.h>
#include<sys/shm.h>

#define N 8 
#define BUF_SIZE 128

typedef struct{
	int id;
	int head,tail;
	char buf[N][BUF_SIZE];
}QUEUE;

#define NUM_SEMAPHORE	3
#define MUTEX	0
#define FULL	1
#define EMPTY	2

#define SEM_KEY 0x2233
#define SHM_KEY 0x6666

#define WHITE	"\033[0;37m"
#define RED	"\033[0;31m"
#define YELLOW	"\033[0;33m"
#define GREEN	"\033[0;32m"
#define PINK	"\033[0;35m"

void create_ipc(void){
	int sem_id,shm_id;
	struct sembuf ops[3];
	QUEUE *q;
	sem_id=semget(SEM_KEY,NUM_SEMAPHORE,IPC_CREAT|IPC_EXCL|0666);
	if(sem_id==-1){
		perror("Create Semaphores");
		exit(1);
	}else{
		printf("Create Semaphores:OK\n");
		ops[0].sem_num=MUTEX;
		ops[0].sem_op=1;
		ops[0].sem_flg=0;
		ops[1].sem_num=FULL;
		ops[1].sem_op=0;
		ops[1].sem_flg=0;
		ops[2].sem_num=EMPTY;
		ops[2].sem_op=N;
		ops[2].sem_flg=0;
	}
	if(semop(sem_id,ops,3)==-1){
		perror("semop");
		exit(1);
	}else{
		printf("Initialize Semaphores:OK\n");
	}

	shm_id=shmget(SHM_KEY,sizeof(QUEUE),IPC_CREAT|IPC_EXCL|0666);
	if(shm_id==-1){
		perror("Create Share Memory");
		exit(1);
	}else{
		printf("Create Share Memory:OK\n");
		q=(QUEUE*)shmat(shm_id,0,0);
		if(q==(QUEUE*)-1){
			perror("Attach Share Memory");
			exit(1);
		}else{
			q->head=q->tail=0;
			printf("Initialize QUEUE:OK\n");
		}
	}
}

void remove_ipc(void){
	int sem_id,shm_id;
	sem_id=semget(SEM_KEY,0,0);
	if(sem_id==-1){
		perror("Get Semaphores");
	}else{
		if(semctl(sem_id,0,IPC_RMID,0)==-1){
			perror("Remove Semaphores");
		}else{
			printf("Remove Semaphores:OK\n");
		}
	}
	
	shm_id=shmget(SHM_KEY,0,0);
	if(shm_id==-1){
		perror("Get Share Memory");
	}else{
		if(shmctl(shm_id,IPC_RMID,0)==-1){
			perror("Remove Share Memory");
		}else{
			printf("Remove Share Memory:OK\n");
		}
	}
}


static int sem_id;
static QUEUE *q;

void get_ipc(void){
	int shm_id;
	sem_id=semget(SEM_KEY,0,0);
	if(sem_id==-1){
		perror("Get Semaphore ID");
		exit(1);
	}
	shm_id=shmget(SHM_KEY,0,0);
	if(shm_id==-1){
		perror("Get Share Memory ID");
		exit(1);
	}
	q=(QUEUE*)shmat(shm_id,0,0);
	if(q==(QUEUE*)-1){
		perror("Attach Share Memory");
		exit(1);
	}
}

void Wait(int sem_num){
	struct sembuf op;
	op.sem_num=sem_num;
	op.sem_op=-1;
	op.sem_flg=0;
	if(semop(sem_id,&op,1)==-1){
		perror("Wait(semaphore)");
		exit(1);
	}
}

void Signal(int sem_num){
	struct sembuf op;
	op.sem_num=sem_num;
	op.sem_op=1;
	op.sem_flg=0;
	if(semop(sem_id,&op,1)==-1){
		perror("Signal(semaphore)");
		exit(1);
	}
}

void produce(char *data){
	int data_id;
	sleep(1+random()%5);
	Wait(MUTEX);
	data_id=q->id++;
	Signal(MUTEX);
	sprintf(data,"PRODUCER-%d ID-%d",getpid(),data_id);
}

void producer(void){
	char data[BUF_SIZE];
	for(;;){
		produce(data);
		Wait(EMPTY);
		Wait(MUTEX);
		strcpy(q->buf[q->tail],data);
		printf(PINK"Producer %d Write Buffer #%d,Data:%s"WHITE"\n",getpid(),q->tail,q->buf[q->tail]);
		q->tail=(q->tail+1)%N;
		Signal(MUTEX);
		Signal(FULL);
	}
}

void consume(char *data){
	sleep(1+random()%5);
}

void consumer(void){
	char data[BUF_SIZE];
	for(;;){
		Wait(FULL);
		Wait(MUTEX);
		strcpy(data,q->buf[q->head]);
		printf(GREEN"Consumer %d Read Buffer #%d,Data:%s"WHITE"\n",getpid(),q->head,q->buf[q->head]);
		q->head=(q->head+1)%N;
		Signal(MUTEX);
		Signal(EMPTY);
		consume(data);
	}
}


int main(int argc,char **argv){
	if(argc>1&&strcmp(argv[1],"create")==0){
		create_ipc();
	}else if(argc>1&&strcmp(argv[1],"remove")==0){
		remove_ipc();
	}else if(argc>1&&strcmp(argv[1],"producer")==0){
		get_ipc();
		producer();
	}else if(argc>1&&strcmp(argv[1],"consumer")==0){
		get_ipc();
		consumer();
	}else{
		printf("Usage: %s {create | remove | producer | consumer }\n",argv[0]);
	}
	return 0;
}












