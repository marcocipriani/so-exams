/*
 *  input:  pathname output file, n num_procs
 *  main:   write decimal int on output file
 *  thread: scan stdin and send to father
 *  signal: print output file
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
 #include <sys/sem.h>
 #include <sys/mman.h>
 #include <sys/semaphore.h>
 #include <fcntl.h>

#define SIZE 4096

int *values; // integers scanned by children
int semid; // semaphore id

char buffer[SIZE];

void handler(int signum){
  system(buffer); // pass the buffer where the first char are 'cat %s'
}

void child_work(){

  int new; // new value read

  int ret; // aux for return values
  struct sembuf oper; // aux for semop

  signal(SIGINT, SIG_IGN); // ignore SIGINT

  while(1){

    ret = scanf("%d", &new);
    if(ret == 0){ printf("Input error\n"); exit(EXIT_FAILURE); }
    printf("Proc #%d is reading %d\n", getpid(), new);

    // first attempt go because sem_num[1] = 1
    // then wait for father to read
    oper.sem_num = 1;
    oper.sem_op = -1;
    oper.sem_flg = SEM_UNDO;
    ret = semop(semid, &oper, 1);
    if(ret == -1) { printf("Semop error\n"); exit(EXIT_FAILURE); }

    printf("Proc #%d is writing %d\n", getpid(), new);
    *values = new;

    // signal to father
    oper.sem_num = 0;
    oper.sem_op = 1;
    oper.sem_flg = SEM_UNDO;
    ret = semop(semid, &oper, 1);
    if(ret == -1) { printf("Semop error\n"); exit(EXIT_FAILURE); }


  }
}

int main(int argc, char const *argv[]) {

  int fd;
  int num_proc;
  FILE *file;

  int ret; // aux for return values
  struct sembuf oper; // aux for semop

  /****************************** USAGE ******************************/
  if (argc != 3) { printf("[Usage]: %s pathname num_proc\n", argv[0]); exit(EXIT_FAILURE); }

  /****************************** INPUT ******************************/
  //file open after child spawning
  num_proc = strtol(argv[2], NULL, 10);

  /****************************** MEMORY ******************************/
  values = mmap(NULL, SIZE, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, 0, 0);
  if (values == NULL) { printf("Error in mmap\n"); exit(EXIT_FAILURE); }

  /****************************** SEMAPHORE ******************************/
  semid = semget(IPC_PRIVATE, 2, IPC_CREAT|0666); // array with 2 semaphores [{0 = father_lock}, {1 = children_lock}]
  if(semid==-1) { printf("Error in semget 2"); exit(EXIT_FAILURE); }

  ret = semctl(semid, 0, SETVAL, 0);
  if(ret==-1) { printf("Error in semctl"); exit(EXIT_FAILURE); }
  ret = semctl(semid, 1, SETVAL, 1);
  if(ret==-1) { printf("Error in semctl"); exit(EXIT_FAILURE); }


  /****************************** SIGNAL ******************************/
  signal(SIGINT,handler);

  /****************************** PROCESSES ******************************/
  printf("Spawing %d processes\n", num_proc);
  for(int i=0; i < num_proc; i++){
    if(fork()) continue;
    else child_work();
  }

  sprintf(buffer, "cat %s", argv[1]); // alternative way to print with terminal command

  /****************************** FATHER WORK ******************************/

  fd = open(argv[1], O_CREAT|O_RDWR, 0666);
  file = fdopen(fd, "r+"); // open file for read and write

  while(1){

    // wait for new int
    oper.sem_num = 0;
    oper.sem_op = -1;
    oper.sem_flg = SEM_UNDO;
wait1:
    ret = semop(semid, &oper, 1);
    if(ret == -1 && errno == EINTR) goto wait1;

    // copy values
    printf("found values is %d\n", *values);
copy:
    ret = fprintf(file, "%d", *values);
    if(ret == -1 && errno == EINTR) goto copy;
    fflush(file);

    // signal the children
    oper.sem_num = 1;
    oper.sem_op = 1;
    oper.sem_flg = SEM_UNDO;
wait2:
    ret = semop(semid, &oper, 1);
    if(ret == -1 && errno == EINTR) goto wait2;
  }

  return 0;
}
