#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <unistd.h>
#define SERVER 1L
struct MsgBuf
{
	long mtype;
	int a[2];	//operands
	char op;	//operator
	pid_t cli_pid;
	//char c;
};

int qid,qflag=0;	//qflag to check if queue is initialised
FILE *fp;	//for file to store output
int fileFlag=0;	//flag to check if fp is initialised

void sighand()	//signal handler for Ctrl-C
{
	char c;
	printf("Are you sure you want to quit?[y/n]\n");
	scanf("%c",&c);
	if (c=='y' || c=='Y')
	{
		if(fileFlag==1)		//to close file ptr is initialised
			fclose(fp);

		if(qflag==1)		//to close queue if initialised
		{
			if(msgctl(qid,IPC_RMID,NULL)<0)
			{
				perror("msgsnd");
				exit(1);
			}
		}
		exit(0);
	}
}

int main(int argc, char const *argv[])
{
	
	struct MsgBuf message;
	
	signal(SIGINT,sighand);	//binds sighand with Ctrl-C

	key_t mkey;
	char *path="/bin/chmod";
	int id='b';

	if((mkey=ftok(path,id))<0)
	{
		perror("ftok");
		exit(1);
	}
	if ((qid=msgget(mkey,IPC_CREAT | 0666))<0)	//r/w permission for user,group
	{
		perror("msgget");
		exit(1);
	}
	qflag = 1;
	printf("qid:%d\n",qid );

	fp = fopen("output.txt","a");	//opens output.txt with append prermission
	if(fp==NULL)
	{
		if(qflag==1)		//to close queue if initialised
		{
			if(msgctl(qid,IPC_RMID,NULL)<0)
			{
				perror("msgsnd");
				exit(1);
			}
		}
		perror("file");
		exit(1);
	}
	while(1)
	{
		if(msgrcv(qid,&message,sizeof(struct MsgBuf)-sizeof(long),SERVER,0)<0)
		{
			perror("msgrcv");
			exit(1);
		}
		printf("message from %d:\t",message.cli_pid );
		fprintf(fp,"message from %d:\t",message.cli_pid );
		switch(message.op)		//do operation based on operator
		{
			case '+': printf("%d\n",message.a[0]+message.a[1] );
					  fprintf(fp,"%d\n",message.a[0]+message.a[1] );
					  break;
			case '-': printf("%d\n",message.a[0]-message.a[1] ); 
					  fprintf(fp,"%d\n",message.a[0]-message.a[1] );
					  break;
			case '*': printf("%d\n",message.a[0]*message.a[1] );
					  fprintf(fp,"%d\n",message.a[0]*message.a[1] );
					  break;
			case '/': printf("%d\n",message.a[0]/message.a[1] );
					  fprintf(fp,"%d\n",message.a[0]/message.a[1] );
					  break;
			default: printf("operator not recognised\n");
		}
	}
	fclose(fp);
	/*if(msgrcv(qid,&message,sizeof(struct MsgBuf)-sizeof(long),SERVER,0)<0)
		{
			perror("msgrcv");
			exit(1);
		}
		printf("%c\n",message.c );*/
	return 0;
}