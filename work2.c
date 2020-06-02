#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#define THREAD_NUMBER 3	//线程数
#define REPEAT_NUMBER 5	//任务数
#define DELAY_TIME_MAX 10	//最大执行时间

void *thread_func(void){
	int thread_id=pthread_self();
	int delay_time=0;
	int count=0;
	int sum=0;
	printf("Thread %d is starting\n",thread_id);
	//每个线程执行REPEAT_NUMBER个任务，每个任务即睡眠一段时间，且时间不超过DELAY_TIME_MAX秒
	for(count=0;count<REPEAT_NUMBER;count++){
		delay_time=(int)(rand()%DELAY_TIME_MAX)+1;
		sleep(delay_time);
		printf("\tThread %d:job %d delay=%d\n",thread_id,count,delay_time);
		sum+=delay_time;
	}
	printf("Thread %d finish,exit %d\n",thread_id,delay_time);
	//线程退出时返回所有任务的总执行时间
	pthread_exit((void*)sum);
}

int main(){
	pthread_t thread[THREAD_NUMBER];
	int no,res;
	void* ret[THREAD_NUMBER];
	srand(time(NULL));
	//创建THREAD_NUMBER个线程
	for(no=0;no<THREAD_NUMBER;no++){
		res=pthread_create(&thread[no],NULL,(void*)thread_func,NULL);
		if(res!=0){
			printf("Create thread %d failed\n",no);
			exit(res);
		}
	}
	printf("Creating threads success\nWaiting for threads to finish...\n");
	//等待THREAD_NUMBER个线程结束并打印其返回值
	for(no=0;no<THREAD_NUMBER;no++){
		res=pthread_join(thread[no],&ret[no]);
		if(!res){
			printf("Thread %d joined,return %d\n",no,(int)ret[no]);			
		}else{
			printf("Thread %d join failed\n",no);
		}
	}
	return 0;
}
