#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
void quit(){
	//收到信号SIGINT后，结束进程
	printf("Process %d received SIGINT,will exit\n",getpid());
	exit(0);
}
int main(){
	int ret;
	if(ret=fork()){
		//父进程睡眠秒后向子进程发送信号SIGINT
		printf("Parent:%d\n",getpid());
		sleep(3);
		kill(ret,SIGINT);
		printf("Parent %d sent SIGINT to child %d\n",getpid(),ret);
		//父进程等待子进程结束执行ps
		wait(0);
		printf("Parent exec ps and then exit\n");
		execlp("ps","ps",NULL);
	}else{
		//设置子进程收到信号SIGINT的处理函数
		printf("Child:%d\n",getpid());
		signal(SIGINT,quit);
		//无限循环
		for(;;){}
	} 
}

