#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
typedef struct{
	int num;
	char *name;
}menber;

static int a=5;

//向线程传递字符串
void thread0(char *arg){
	char *str;
	str=arg;
	printf("Thread0 received parameter passed from main is %s\n",str);
}

//向线程传递结构体
void thread1(void *arg){
	menber* temp=(menber*)arg;
	printf("Thread1 menber->num=%d\n",temp->num);
	printf("Thread1 menber->name=%s\n",temp->name);
}

//通过全局变量向线程传参
void thread2(void* arg){
	printf("Thread2 a=%d\n",a);
}

int main(){
	int res;
	pthread_t id[3];

	char *str1="Hello!";
	res=pthread_create(&id[0],NULL,(void*)thread0,(void*)str1);
	if(res!=0){
		printf("Thread0 id not create!\n");
		exit(1);
	}
	
	menber *p=(menber*)malloc(sizeof(menber));
	p->num=1;
	p->name="Robben!";
	res=pthread_create(&id[1],NULL,(void*)thread1,(void*)p);
	if(res!=0){
		printf("Thread1 id not create!\n");
		exit(1);
	}

	res=pthread_create(&id[2],NULL,(void*)thread2,NULL);
	if(res!=0){
		printf("Thread2 id not create!\n");
		exit(1);
	}
	
	for(int i=0;i<3;i++){
		pthread_join(id[i],NULL);
	}
	return 0;
}
