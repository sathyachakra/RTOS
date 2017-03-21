#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

int main(int argc, char const *argv[])
{
	pid_t cpid;																	//child pid
	int fd[2],nbytes,ret,len,i,count=0/*,writeBuffer[500],readBuffer[500]*/;	//desc for pipe, num of bytes, return val, num of lines
	char writeBuffer[500],readBuffer[500],receivedData[500];
	DIR *dp;
	struct dirent *dirEntry;
	//char path[]="/home/sathya/Desktop/summa";
	
	char path[100];
	//scanf("%s",path);
	strcpy(path,argv[1]);
	pipe(fd);		//create pipe. make fd as the read and write ends

	cpid = fork();

	if(cpid<0)
	{
		perror("fork");
		exit(-1);
	}


	//Process that does the "wc -l" command
	else if(cpid!=0)											//Parent process
	{
		close(fd[1]);
		wait(NULL);
		nbytes = read(fd[0],readBuffer,sizeof(readBuffer));		//read from the pipe
		for (i = 0; i < strlen(readBuffer); ++i)
		{
			if ((readBuffer[i]=='\n')||(readBuffer[i]==' '))
			{
				count++;
			}
		}
		printf("%d\n",count-2 );								//since first 2 lines are junk, count-2
	}

	


	//process that does the "ls" command
	else														//Child process
	{
		close(fd[0]);
		//printf("in child\n");
		dp = opendir(path);										//access directory present in path variable
		while((dirEntry=readdir(dp))!=NULL)
		{
			strcat(writeBuffer,dirEntry->d_name);				//get file names present in directory
			len = strlen(writeBuffer);
			writeBuffer[len] = '\n';
		}
		write(fd[1],writeBuffer,strlen(writeBuffer)+1);			//write file names in the write end of pipe
	}

	return 0;
}
