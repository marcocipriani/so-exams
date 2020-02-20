/*
 *  input:      file_pathname F, string N
 *  main/procA: -
 *  threadA:    scan S stdin, write S in mem[i]
 *  procB:      -
 *  threadB:    write mem[i] in F
 *  signal:     A: redirect to B; B: print F
 */

/*
 *  N [0,1,...,N-1] for new content; 0: no, 1: new string
 *  R [0,1,...,N-1] for have read; 0: no, 1: have read
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/semaphore.h>
#include <fcntl.h>

#define SIZE 4096

char *pathname;
int num_threads;
char **mem; // shared memory for new strings

int sdN, sdR;

void parent_handler(){
  // redirect signal to child
}

void child_handler(){
  // print F
}

void threadA_work(){
  // while
    // scan S from stdin
    scanf("%s", mem)
    // signal N[i]
    // wait R[i]
}

void threadB_work(){
  // while
    // wait N[i]
    // write in F
    // signal R[i]
}

void child_work(){

  // create N thread
}

int main(int argc, char const *argv[]) {

  pid_t pid;

  /****************************** USAGE ******************************/
  if(argc != 3){ printf("Error\n"); exit(-1); }

  /****************************** INPUT ******************************/
  pathname = argv[1];
  num_threads = strtol(argv[2], NULL, 10);
  printf("Planned to spawn %d threads\n", num_threads);

  /****************************** MEMORY ******************************/
  // buffer [4KB * num_threads]
  mem = malloc(num_threads * sizeof(char *));
  if(mem == NULL){ ERROR }

  for(int i=0; i < num_threads; i++){
    mem[i] = mmap(NULL, SIZE, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, 0, 0);
    if(mem[i] == NULL){ ERROR }
  }

  /****************************** SEMAPHORE ******************************/
  // sem N
  sdN = semget(IPC_PRIVATE, num_threads, IPC_CREAT|0666);
  ERROR
  for(i=0; i < num_threads; i++){
    semctl(sdN, i, SETVAL, 0);
    ERROR
  }
  // sem R
  sdR = semget(IPC_PRIVATE, num_threads, IPC_CREAT|0666);
  ERROR
  for(i=0; i < num_threads; i++){
    aux = semctl(sdR, i, SETVAL, 0);
    if(aux == -1) ERROR
  }

  /****************************** SIGNAL ******************************/
  // a_handler, b_handler during fork()

  /****************************** THREAD + PROC ******************************/
  // create N thread
  // fork child
  pid = fork();
  if(pid == -1) ERROR
  if(pid == 0){
    signal(SIGINT, child_handler);
  } else {
    signal(SIGINT, parent_handler);
  }

  /****************************** FATHER WORK ******************************/
  // pause

  return 0;
}
