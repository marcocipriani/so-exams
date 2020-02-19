/*
*   input:         n >= 1, n stringa di cifre decimali
*   main thread:   attesa indefinita di interi da stdin e genera i record per un thread selezionato
*   n thread:      ognuno a turno dovrà inserire in una propria lista basata su memoria dinamica un record
*   signal:        SIGINT print all the threads' lists
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

// list
typedef struct data{
  int val;
  struct data *next; // next element
};

int num_threads; // it has to be global for the signal handler

struct data *lists; // threads' lists
pthread_mutex_t father_lock; // father's mutex
pthread_mutex_t child_lock; // second mutex

int parse;

void* worker(void* me){
  struct data* aux;
  int id = (int)me; // useful for selecting proper space in lists
  int ret; // aux for locking return values

  printf("Thread %d is processing\n", id);

  while (1) {
    aux = malloc(sizeof(struct data));  // ?????????????????????????????????????????????????????????????????
    if (aux == NULL) { printf("Error in thread malloc\n"); exit(EXIT_FAILURE); }

    // wait untile lock becames available
    ret = pthread_mutex_lock(&child_lock);
    if (ret != 0) { printf("Locking error for thread %d\n", id); exit(EXIT_FAILURE); }

    printf("Thread %d: new value is %d\n", id, parse);
    aux->val = parse;

    // signal
    ret = pthread_mutex_unlock(&father_lock);
    if (ret != 0) { printf("Unlocking error for thread %d\n", id); exit(EXIT_FAILURE); }

    aux->next = lists[id].next;
    lists[id].next = aux;
  }

  return 0; // ?????????????????????????????????????????????????????????????????
}

void printer(int signum){
  struct data printaux;
  int i;

  printf("Signal #%d\n", signum);

  for (i=1; i < num_threads; i++){
    printaux = lists[i];
    printf("List %d: ", i);

    while (printaux.next) {
      printf("%d ", printaux.val);
      printaux = (*printaux).next);
    }
    printf("\n");
  }
  printf("ciao\n");
}


int main(int argc, char const *argv[]) {

  int i; // aux for thrads
  int ret; // aux for locking return values
  pthread_t tid; // id for creating threads

  /****************************** USAGE ******************************/
  if (argc != 2) { printf("[Usage] %s num_threads\n", argv[0]); exit(EXIT_FAILURE); }

  printf("I'm the father\n");

  /****************************** INPUT ******************************/
  num_threads = strtol(argv[1], NULL, 10);
  if (num_threads <= 0) { printf("Invalid num_threads\n"); exit(EXIT_FAILURE); }
  printf("Planned to spawn %d threads\n", num_threads);

  /****************************** MEMORY ******************************/
  lists = malloc(num_threads * sizeof(struct data));
  if (lists == NULL) { printf("Error in malloc\n"); exit(EXIT_FAILURE); }

  // initialization
  for (i=0; i < num_threads; i++) {
    lists[i].val = -1;
    lists[i].next = NULL;
  }

  /****************************** SEMAPHORES ******************************/
  ret = pthread_mutex_init(&father_lock, NULL);
  if (ret != 0) { printf("Init error father's semaphore\n"); exit(EXIT_FAILURE); }
  ret = pthread_mutex_init(&child_lock, NULL);
  if (ret != 0) { printf("Init error child's semaphore\n"); exit(EXIT_FAILURE); }

  ret = pthread_mutex_lock(&child_lock);
  if (ret != 0) { printf("Lock error child's semaphore\n"); exit(EXIT_FAILURE); }

  /****************************** SIGNAL ******************************/
/*
  sigfillset(&set);
  act.*sa_sigaction = printer; // sa_sigcation or __sigaction_u ?
  act.sa_mask = set;
  act.sa_flags = 0;
  sigaction(SIGINT, &act, NULL);
  */

  signal(SIGINT, printer);

  /****************************** THREADS ******************************/
  for (i=0; i<num_threads; i++) {
    ret = pthread_create(&tid, NULL, worker, (void*)i); // arg is void*
    if (ret != 0) { printf("Error creating thread %d\n", i); exit(EXIT_FAILURE); }
  }

  /****************************** FATHER WORK ******************************/

  while (1) {
step1:
    ret = pthread_mutex_lock(&father_lock);
    if(ret != 0 && errno == EINTR) goto step1;

step2:
    ret = scanf("%d", &parse);
    if(ret == EOF) goto step2;
    if(ret == 0) { printf("Wrong input\n"); exit(EXIT_FAILURE); }

step3:
    ret = pthread_mutex_unlock(&child_lock);
    if(ret != 0 && errno == EINTR) goto step3;

  }

  return 0;
}
