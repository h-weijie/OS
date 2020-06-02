//share memory
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>

#include<sys/ipc.h>
#include<sys/sem.h>
#include<sys/shm.h>

#define N 8 //缓冲区能容纳的数据个数
#define BUF_SIZE 128 //数据大小

//缓冲区队列
typedef struct{
	int id;
	int head,tail;
	char buf[N][BUF_SIZE];
}QUEUE;

#define NUM_SEMAPHORE	3	//信号量个数
#define MUTEX	0	//MUTEX用于缓冲区队列的互斥使用
#define FULL	1	//FULL表示缓冲区中数据个数
#define EMPTY	2	//EMPTY表示缓冲区中空闲量

#define SEM_KEY 0x2233
#define SHM_KEY 0x6666

#define WHITE	"\033[0;37m"
#define RED		"\033[0;31m"
#define YELLOW	"\033[0;33m"
#define GREEN	"\033[0;32m"
#define PINK	"\033[0;35m"

//创建信号量与共享内存并初始化
void create_ipc(void){
	int sem_id,shm_id;
	struct sembuf ops[3];
	QUEUE *q;
	sem_id=semget(SEM_KEY,NUM_SEMAPHORE,IPC_CREAT|IPC_EXCL|0666);//创建信号量
	if(sem_id==-1){
		perror("Create Semaphores");
		exit(1);
	}else{
		printf("Create Semaphores:OK\n");
		//MUTEX初始为1
		ops[0].sem_num=MUTEX;
		ops[0].sem_op=1;
		ops[0].sem_flg=0;
		//FULL初始为0
		ops[1].sem_num=FULL;
		ops[1].sem_op=0;
		ops[1].sem_flg=0;
		//EMPTY初始为N
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

	shm_id=shmget(SHM_KEY,sizeof(QUEUE),IPC_CREAT|IPC_EXCL|0666);//创建共享内存
	if(shm_id==-1){
		perror("Create Share Memory");
		exit(1);
	}else{
		printf("Create Share Memory:OK\n");
		q=(QUEUE*)shmat(shm_id,0,0);//获得共享内存的指针
		if(q==(QUEUE*)-1){
			perror("Attach Share Memory");
			exit(1);
		}else{
			q->head=q->tail=0;//初始化缓冲区队列
			printf("Initialize QUEUE:OK\n");
		}
	}
}

//删除信号量与共享内存
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
//获取信号量与共享内存设置到全局变量中
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

//信号量wait操作
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

//信号量signal操作
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

//生产数据
void produce(char *data){
	int data_id;
	sleep(1+random()%5);
	Wait(MUTEX);
	data_id=q->id++;
	Signal(MUTEX);
	sprintf(data,"PRODUCER-%d ID-%d",getpid(),data_id);
}

//生产者进程
void producer(void){
	char data[BUF_SIZE];
	for(;;){
		produce(data);
		Wait(EMPTY);
		Wait(MUTEX);
		strcpy(q->buf[q->tail],data);
		printf(RED"Producer %d Write Buffer #%d,Data:%s"WHITE"\n",getpid(),q->tail,q->buf[q->tail]);
		q->tail=(q->tail+1)%N;
		Signal(MUTEX);
		Signal(FULL);
	}
}

//消费数据
void consume(char *data){
	sleep(1+random()%5);
}

//消费者进程
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
	//根据不同的程序名执行不同的操作
	if(strstr(argv[0],"create")){
		create_ipc();
	}else if(strstr(argv[0],"remove")){
		remove_ipc();
	}else if(strstr(argv[0],"producer")){
		get_ipc();
		producer();
	}else if(strstr(argv[0],"consumer")){
		get_ipc();
		consumer();
	}else{
		printf("Use name: {create | remove | producer | consumer }\n");
	}
	return 0;
}
