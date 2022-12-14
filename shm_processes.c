
// Kuira Edwards @02942519

#include  <stdio.h>
#include  <stdlib.h>
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syst/mman.h>
sem_t *mutex;

void  ChildProcess(int *SharedMemory) {
  int accountID, num;
  srand(getpid());

  for(int i = 0; i < 25; i++) {
    sleep(rand() % 6);
    sem_wait(mutex);
    accountID = *SharedMemory;
    num = rand() % 51;
    printf("Poor Student needs $%d\n", num);

    if(num <= accountID) {
      accountID -= num;
      printf("Poor Student: Withdraws $%d / Balance = $%d\n", num, accountID);
      *SharedMemory = accountID;
    }
    else {
      printf("Poor Student: Not Enough Cash ($%d)\n", accountID);
    }
    sem_post(mutex);
  }
}

void ParentProcess(int *SharedMemory) {
  int accountID, num;
  srand(getpid());

  for(int i = 0; i < 25; i++) {
    sleep(rand() % 6);
    sem_wait(mutex);
    accountID = *SharedMemory;

    if(accountID <= 100) {
      num = rand() % 101;
      if(num % 2 == 0) {
        accountID += num;
        printf("Dear old Dad: Deposits $%d / Balance = $%d\n", num, accountID);
      }
      else {
        printf("Dear old Dad: Doesn't have any money to give\n");
      }
      *SharedMemory = accountID;
    }
    else {
      printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", accountID);
    }
    sem_post(mutex);
  }
}

int  main(int  argc, char *argv[])
{
     int    ShmID;
     int    *ShmPTR;
     pid_t  pid;
     int    status;

     if ((mutex = sem_open("examplesemaphore", O_CREAT, 0644, 1)) == SEM_FAILED) {
          perror("semaphore initialization");
          exit(1);
     }

     ShmID = shmget(IPC_PRIVATE, 4*sizeof(int), IPC_CREAT | 0666);
     if (ShmID < 0) {
          printf("*** shmget error (server) ***\n");
          exit(1);
     }
     printf("Server has received a shared memory of four integers...\n");

     ShmPTR = (int *) shmat(ShmID, NULL, 0);
     if (*ShmPTR == -1) {
          printf("*** shmat error (server) ***\n");
          exit(1);
     }
     printf("Server has attached the shared memory...\n");

     *ShmPTR = 0;
     //printf("Server has filled %d %d %d %d in shared memory...\n", ShmPTR[0], ShmPTR[1], ShmPTR[2], ShmPTR[3]);

     printf("Main is about to fork a child process...\n");
     pid = fork();
     if (pid < 0) {
          printf("*** fork error (server) ***\n");
          exit(1);
     }
     else if (pid == 0) {
          ClientProcess(ShmPTR);
          exit(0);
     }
     else {
       ParentProcess(ShmPTR);
     }
     wait(&status);
     printf("Main has detected the completion of its child...\n");
     shmdt((void *) ShmPTR);
     printf("Main has detached its shared memory...\n");
     shmctl(ShmID, IPC_RMID, NULL);
     printf("Main has removed its shared memory...\n");
     printf("Main exits...\n");
     exit(0);
}