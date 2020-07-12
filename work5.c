#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>

#include<sys/ipc.h>
#include<sys/sem.h>
#include<sys/shm.h>

#define WHITE	"\033[0;37m"
#define RED	"\033[0;31m"
#define YELLOW	"\033[0;33m"
#define GREEN	"\033[0;32m"

#define SEM_KEY 0x2233
#define SHM_KEY 0x6666

#define NUM_SEMAPHORE	4//信号量个数
#define mutexStation	0
#define suspend_A	1
#define suspend_B	2
#define suspend_C	3

//工作台，记录A、B与空位的个数
typedef struct{
	int count_A,count_B,empty;
}Station;

static int sem_id;
static Station *s;//共享内存

//信号量初始化，将sem_num初始为value
void init_sem(int sem_num,int value){
	struct sembuf op;
	op.sem_num=sem_num;
	op.sem_op=value;
	op.sem_flg=0;
	if(semop(sem_id,&op,1)==-1){
		perror("init(semaphore)");
		exit(1);
	}
}

//在Linux内核中创建信号量与共享内存
void create_ipc(void){
	int shm_id;
	sem_id=semget(SEM_KEY,NUM_SEMAPHORE,IPC_CREAT|IPC_EXCL|0666);
	if(sem_id==-1){
		perror("Create Semaphores");
		exit(1);
	}else{
		printf("Create Semaphores:OK\n");
		//初始化信号量
		init_sem(mutexStation,1);
		init_sem(suspend_A,0);
		init_sem(suspend_B,0);
		init_sem(suspend_C,0);
		printf("Initialize Semaphores:OK\n");
	}

	shm_id=shmget(SHM_KEY,sizeof(Station),IPC_CREAT|IPC_EXCL|0666);
	if(shm_id==-1){
		perror("Create Share Memory");
		exit(1);
	}else{
		printf("Create Share Memory:OK\n");
		s=(Station*)shmat(shm_id,0,0);
		if(s==(Station*)-1){
			perror("Attach Share Memory");
			exit(1);
		}else{
			s->count_A=s->count_B=0;//初始化工作站，A与B的个数初始为零，空位为12
			s->empty=12;
			printf("Initialize STATION:OK\n");
		}
	}
}

//删除内核中的信号量与共享内存
void remove_ipc(void){
	int shm_id;
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

//获取内核中的信号量与共享内存，并设置到全局变量中
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
	s=(Station*)shmat(shm_id,0,0);
	if(s==(Station*)-1){
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

//生产部件，类型为tyoe，个数为num
void produce(char type,int num){
	sleep(1+random()%num);
}

//检查工作站状态并打印
void pstation(){
	if(s->count_A>=0&&s->count_B>=0&&s->count_A+s->count_B<=12){	//检查工作站状态是否正确
		printf("[");
		int i;
		for(i=0;i<s->count_A;i++){ printf("A"); }
		for(i=0;i<s->count_B;i++){ printf("B"); }
		for(i=0;i<s->empty;i++){ printf("-"); }
		printf("]\n");
	}else{
		printf("Station error\n");
		exit(1);
	}
}

//将个数为num，类型tyoe的部件放入工作站
void put(char type,int num){
	sleep(1+random()%num);
}

//同时取出numA个partA与numB个partB
void takeAB(int numA,int numB){
	int num=numA+numB;
	sleep(1+random()%num);
}

//工人A的进程
void workA(void){
	int i;
	get_ipc();
	for(;;){
		Wait(mutexStation);
		if(s->count_A<=7&&s->empty>=2){
			produce('A',2);
			put('A',2);
			s->count_A=s->count_A+2;
			s->empty=s->empty-2;
			printf(RED"WorkA put two A to Station"WHITE);
			pstation();
			Signal(mutexStation);
			Signal(suspend_C);
		}else{
			Signal(mutexStation);
			Wait(suspend_A);
		}
	}
}

//工人B的进程
void workB(void){
	int i;
	get_ipc();
	for(;;){
		Wait(mutexStation);
		if(s->count_B<=7&&s->empty>=1){
			produce('B',1);
			put('B',1);
			s->count_B=s->count_B+1;
			s->empty=s->empty-1;
			printf(YELLOW"WorkB put one B to Station"WHITE);
			pstation();
			Signal(mutexStation);
			Signal(suspend_C);
		}else{
			Signal(mutexStation);
			Wait(suspend_B);
		}
	}
}

//工人C的进程
void workC(void){
	int i;
	get_ipc();
	for(;;){
		Wait(mutexStation);
		if(s->count_A>=4&&s->count_B>=3){
			takeAB(4,3);
			s->count_A=s->count_A-4;
			s->count_B=s->count_B-3;
			s->empty=s->empty+7;
			printf(GREEN"WorkC take 4A3B from Station"WHITE);
			pstation();
			produce('C',1);
			Signal(mutexStation);
			Signal(suspend_A);
			Signal(suspend_B);
		}else{
			Signal(mutexStation);
			Wait(suspend_C);
		}
	}
}

int main(int argc,char **argv){
	//根据程序名运行相应操作
	if(strstr(argv[0],"create")){
		create_ipc();
	}
	if(strstr(argv[0],"remove")){
		remove_ipc();
	}
	if(strstr(argv[0],"workA")){
		workA();
	}	
	if(strstr(argv[0],"workB")){
		workB();
	}
	if(strstr(argv[0],"workC")){
		workC();
	}
	return 0;
}
