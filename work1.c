#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
void quit(){
	printf("Process %d received SIGINT,will exit\n",getpid());
	exit(0);
}
int main(){
	int ret;
	if(ret=fork()){
		printf("Parent:%d\n",getpid());
		sleep(3);
		kill(ret,SIGINT);
		printf("Parent %d sent SIGINT to child %d\n",getpid(),ret);
		wait(0);
		printf("Parent exec ps and then exit\n");
		execlp("ps","ps",NULL);
	}else{
		printf("Child:%d\n",getpid());
		signal(SIGINT,quit);
		for(;;){}
	} 
}

