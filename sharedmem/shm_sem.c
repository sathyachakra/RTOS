#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <semaphore.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define SMSIZE 100    //size of shared memory


int main(int argc, char const *argv[])
{
  key_t key;                                  //key to generate sharedmem
  char *shm;                                  //value to be stored in shared mem
  //char *s;
  int shmid,ret_status;                       //shared mem id, return status
  char loc[] = "/media/sathya/Dump/IIITB/";   //path to be passed to fotk
  pid_t pid;                                  //process id
  sem_t *sem1,*sem2;                          //semaphore for mutual exclusion

  key = ftok(loc,'a');  //unique key
  

  if((shmid = shmget(key,SMSIZE,IPC_CREAT | 0666))<0) //create/get shared memory
  {
    perror("shmget");
    exit(1);
  }

  if((shm = shmat(shmid,NULL,0)) == (char *)-1)       //attach shared mem with shm
  {
    perror("shmat");
    exit(1);
  }

  sem1 = sem_open("shmsem1",O_CREAT | O_EXCL, 0666, 1); //create sempahore
  sem_unlink("shmsem1");                                //prevents sem from existing forever
  sem2 = sem_open("shmsem2",O_CREAT | O_EXCL, 0666, 0); //create semaphore
  sem_unlink("shmsem2");                                //prevents sem from existing forever

  printf("semaphore created\n");
  if((pid=fork())<0)           //error in forking
  {
    shmdt(shm);
    shmctl(shmid,IPC_RMID,0); //remove shared mem
    sem_destroy(sem1);        //destroy semahore
    sem_destroy(sem2);
    perror("fork");
    exit(1);
  }
  else if(pid == 0)           //child process
  {
    sem_wait(sem1);           //wait operation
    printf("child:\t");
    fgets(shm,SMSIZE,stdin);
    /*s = shm;
    s+=strlen(shm);
    *s = '\0';*/
    sem_post(sem2);           //signal operation
  }
  else
  {
    sem_wait(sem2);           //wait operation
    printf("parent:\t");
    fputs(shm,stdout);
    sem_post(sem1);           //signal operation
    shmdt(shm);               //detatch shm
    shmctl(shmid,IPC_RMID,0); //remove sharedmem
    sem_destroy(sem1);        //destroy sempahore
    sem_destroy(sem2);
    printf("semaphore destroyed\n");
  }

  return 0;
}