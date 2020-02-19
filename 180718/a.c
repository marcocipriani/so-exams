/*
*   input:  n pathname
*   main:   wait, pause
*   thread: read from pathanme, write in meme, pause
*   signal: main print mem
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>

#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>

#include <errno.h>


#define SIZE 4096

void ** mem; // whole memory for all the processes
char * buffer; // pointer to mem
int sd;
int i;

int num_proc;

void child_work(const char *pathname){

  FILE *f;

  int fd;
  struct sembuf oper;

  signal(SIGINT, SIG_IGN);

  //open
  fd = open(pathname, O_RDONLY|O_CREAT|0666);
  if(fd == -1) { printf("Error in open"); exit(EXIT_FAILURE); }
  f = fdopen(fd,"r");

  // read from pathname
  // write in mem
  while(fscanf(f, "%s", buffer) != EOF){
    buffer += strlen(buffer)+1;
  }

  // signal(sem_child)
  oper.sem_num = 0;
  oper.sem_op = 1;
  oper.sem_flg = SEM_UNDO;
  semop(sd, &oper, 1);

  // while(1) pause
  while(1) pause();

}

void handler(){
  // print mem
  for(i = 1; i <= num_proc; i++){
    buffer = mem[i];
    while (strcmp(buffer, "\0") != 0) {
      printf("%s\n", buffer);
      buffer += strlen(buffer)+1;
    }
  }
}

int main(int argc, char const *argv[]) {



  int ret; // aux for return value
  int pid; // aux for fork
  struct sembuf oper; // aux for semop


  /***************************** USAGE *****************************/
  if(argc < 2) { printf("[Usage] %s pathname_1 ... pathname_N\n", argv[0]); exit(EXIT_FAILURE); }

  /***************************** INPUT *****************************/
  num_proc = argc - 1;
  // n pathname

  /***************************** MEMORY *****************************/
  mem = malloc(num_proc * sizeof(void *));
  if(mem == NULL) { printf("Error in malloc\n"); exit(EXIT_FAILURE); }

  /***************************** SEMAPHORE *****************************/
  sd = semget(IPC_PRIVATE, 1, IPC_CREAT|0666);
  if(sd == -1) { printf("Error in semget\n"); exit(EXIT_FAILURE); }

  ret = semctl(sd, 0, SETVAL, 0);
  if(ret == -1) { printf("Error in semctl"); exit(EXIT_FAILURE); }

  /***************************** SIGNAL *****************************/
  signal(SIGINT, handler);

  /***************************** FORK *****************************/
  for(i = 1; i <= num_proc; i++){

    mem[i] = mmap(NULL, SIZE, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, 0, 0);
    if(mem[i] == NULL) { printf("Error in Mmap\n"); exit(EXIT_FAILURE); }

    buffer = mem[i];

    pid = fork();
    if(pid == -1) { printf("Error in fork %d\n", i); exit(EXIT_FAILURE); }
    if(pid == 0){
      child_work(argv[i]);
    } else {
      continue;
    }

  }

  /***************************** FATHER WORK *****************************/
  // wait(sem_child)
  oper.sem_num = 0;
  oper.sem_op = -num_proc;
  oper.sem_flg = SEM_UNDO;
wait:
  ret = semop(sd, &oper, 1);
  if(ret == -1){
    if(errno == EINTR) goto wait;
    else { printf("Error in wait\n"); exit(EXIT_FAILURE); }
  }

  while(1) pause();

  return 0;
}
