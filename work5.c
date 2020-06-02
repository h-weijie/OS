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
#define PINK	"\033[0;35m"

#define SEM_KEY 0x2233
#define SHM_KEY 0x6666

#define NUM_SEMAPHORE	5
#define MUTEX	0
#define EMPTYA	1
#define EMPTYB	2
#define FULLA	3
#define FULLB	4

#define N 12

typedef struct{
	int numA,numB;
}Station;

static int sem_id;
static Station *s;

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

void create_ipc(void){
	int shm_id;
	sem_id=semget(SEM_KEY,NUM_SEMAPHORE,IPC_CREAT|IPC_EXCL|0666);
	if(sem_id==-1){
		perror("Create Semaphores");
		exit(1);
	}else{
		printf("Create Semaphores:OK\n");
		init_sem(MUTEX,1);
		init_sem(EMPTYA,8);
		init_sem(EMPTYB,4);
		init_sem(FULLA,0);
		init_sem(FULLB,0);
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
			s->numA=s->numB=0;
			printf("Initialize STATION:OK\n");
		}
	}
}

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

void produce(char type,int num){
	sleep(1+random()%5);
	//for(int i=0;i<num;i++){ sleep(1+random()%5); }
}

void pstation(){
	printf("[");
	int i;
	for(i=0;i<s->numA;i++){ printf("A"); }
	for(i=0;i<s->numB;i++){ printf("B"); }
	int blank=N-s->numA-s->numB;
	for(i=0;i<blank;i++){ printf("-"); }
	printf("]\n");
}

void put(char type,int num){
	if(type=='A'){
		s->numA+=num;
	}
	if(type=='B'){
		s->numB+=num;
	}
}

void take(char type,int num){
	if(type=='A'){
		s->numA-=num;
	}
	if(type=='B'){
		s->numB-=num;
	}	
}

void workA(void){
	int i;
	get_ipc();
	for(;;){
		produce('A',2);
		for(i=0;i<2;i++){ Wait(EMPTYA); }
		Wait(MUTEX);
		put('A',2);
		printf(RED"WorkA put two A to Station"WHITE);
		pstation();
		Signal(MUTEX);
		for(i=0;i<2;i++){ Signal(FULLA); }
	}
}

void workB(void){
	int i;
	get_ipc();
	for(;;){
		produce('B',1);
		Wait(EMPTYB);
		Wait(MUTEX);
		put('B',1);
		printf(YELLOW"WorkB put one B to Station"WHITE);
		pstation();
		Signal(MUTEX);
		Signal(FULLB);
	}
}

void workC(void){
	int i;
	get_ipc();
	for(;;){
		for(i=0;i<4;i++){ Wait(FULLA); }
		for(i=0;i<3;i++){ Wait(FULLB); }
		Wait(MUTEX);
		take('A',4);
		take('B',3);
		printf(GREEN"WorkC take 4A3B from Station"WHITE);
		pstation();
		Signal(MUTEX);
		for(i=0;i<4;i++){ Signal(EMPTYA); }
		for(i=0;i<3;i++){ Signal(EMPTYB); }
		produce('C',1);
	}
}

int main(int argc,char **argv){
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
