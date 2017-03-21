#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <unistd.h>

struct MsgBuf
{
	long mtype;
	int a[2];	//operands
	char op;	//operator
	pid_t cli_pid;
	//char c;
};

void sighand()	//signal handler for Ctrl-C
{
	char c;
	printf("Are you sure you want to quit?[y/n]\n");
	scanf("%c",&c);
	if (c=='y' || c=='Y')
	{
		exit(0);
	}
}

int main(int argc, char const *argv[])
{
	struct MsgBuf message;
	key_t mkey;
	int qid,id='b';
	char *path = "/bin/chmod";

	signal(SIGINT,sighand);	//binds sighand with Ctrl-C

	mkey = ftok(path,id);	//generates key
	if(mkey<0)	//error in generating key
	{
		perror("ftok");
		exit(1);
	}
	if((qid=msgget(mkey,IPC_CREAT | 0666))<0)	//gets/creates queue. gives user,group permission to r/w
	{
		perror("msgget");
		exit(1);
	}
	printf("enter message:\t");
	scanf("%d %d %c",&message.a[0],&message.a[1],&message.op);
	message.mtype=1;	//message type helps in identifying the message
	message.cli_pid = getpid();
	/*message.c='i';
	message.mtype = 1;*/

	if(msgsnd(qid,&message,sizeof(struct MsgBuf)-sizeof(long),0)<0)	//sends msg stored in struct "message"
		exit(1);

	printf("\nmessage sent\n");

	return 0;
}
