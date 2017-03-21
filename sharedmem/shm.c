#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>

#define SMSIZE 100    //shared mem size


int main(int argc, char const *argv[])
{
  key_t key;            //key for creating shared mem
  char *shm;            //shared memory 
  char *s;
  int shmid,ret_status; //shared mem id, return status
  char loc[] = "/media/sathya/Dump/IIITB/";
  pid_t pid;            //process id from fork
  

  key = ftok(loc,'a');  //create key


  if((shmid = shmget(key,SMSIZE,IPC_CREAT | 0666))<0)   //get a shared mem id(if exists) or create one
  {
    perror("shmget");
    exit(1);
  }

  if((shm = shmat(shmid,NULL,0)) == (char *)-1)         //attach shm to shared mem
  {
    perror("shmat");
    exit(1);
  }

  if(!(pid = fork()))               //child process
  {
    printf("child:\t");
    fgets(shm,SMSIZE,stdin);
    //memcpy(shm,"Hello world",11);
    //printf("%ld\n",strlen(shm) );
    //s = shm;
    //s+=strlen(shm);
  }

  else if(pic>0)                    //parent process
  {
    waitpid(pid,&ret_status,0);     //wait for child process to end for mutual exclusion
    if(!ret_status)                 //if child exited successfully
    {
      printf("parent:\t");
      fputs(shm,stdout);            //access shared mem
      /*for(s = shm; *s != '\0'; s++)
      printf("%c", *s);*/
    }
    else  //error
    {
      perror("wait");
      exit(1);
    }
    shmdt(shmid);             //detach
    shmctl(shmid,IPC_RMID,0); //remove shared mem
  }
  else
  {
    shmdt(shmid);
    shmctl(shmid,IPC_RMID,0);
    perror("fork ");
    exit(1);
  }
  

  return 0;
}