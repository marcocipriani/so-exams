/******************** STRUCT VERSION ********************/

/*
 *  input:  N strings
 *  main:   pause
 *  thread: scan stdin, strcmp string and eventually counter++
 *  signal: print counters on output.txt
 */

/*
 *  SEM = [0 = write still pending, 1 = write performed], set initially to 1
 */

#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/sem.h>
#include <semaphore.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 128 // max input string size

int num_threads;
int fd; // file descriptor of output.txt
int sd;

int i; // aux for cycle
int ret; // aux for return values

struct string *strings;
struct string{
    char *text;
    int counter;
};

void handler(int signo){

printf("Receveid SIGINT\n");

    for(i=0; i < num_threads; i++){
        dprintf(fd, "[%d] %s\n", strings[i].counter, strings[i].text);
    }

    exit(EXIT_SUCCESS);
}

void *worker(void *num){

    int me = (int)num;
    char buffer[SIZE];
    int hit;

    struct sembuf oper;

    while(1) {
wait:
        oper.sem_num = 0;
        oper.sem_op = -1;
        oper.sem_flg = 0;
        ret = semop(sd, &oper, 1);
        if(ret == -1){
            if(errno == EINTR) goto wait;
            else{ fprintf(stderr, "Error in wait\n"); exit(EXIT_FAILURE); }
        }

printf("Child %d is waiting to scan\n", me);

scan:
        ret = scanf("%s", buffer);
        if(ret == -1){ fprintf(stderr, "Error in scan\n"); exit(EXIT_FAILURE); }
        if(ret == 0) goto scan;
printf("Input is: %s\n", buffer);

        for(i=0; i < num_threads; i++){
            hit = strcmp(strings[i].text, buffer);
            if(hit != 0){
printf("\t[%d string] not found\n", i);
                continue;
            }
printf("\t[%d string] FOUND\n", i);
            strings[i].counter = strings[i].counter + 1;
            break;
        }

signal:
        oper.sem_num = 0;
        oper.sem_op = 1;
        oper.sem_flg = 0;
        ret = semop(sd, &oper, 1);
        if(ret == -1){
            if(errno == EINTR) goto signal;
            else{ fprintf(stderr, "Error in signal\n"); exit(EXIT_FAILURE); }
        }
    }

}

int main(int argc, char const *argv[]) {

    pthread_t tid;

    /******************** USAGE ********************/
    if(argc < 2){ fprintf(stderr, "Usage: %s string_1 [string_2 ... string_N]\n", argv[0]); exit(EXIT_FAILURE); }

    /******************** INPUT ********************/
    // num_threads
    num_threads = argc - 1;
printf("Planned to spawn %d\n", num_threads);

    /******************** MEMORY ********************/
    // strings
    strings = malloc(num_threads * sizeof(struct string));
    if(strings == NULL){ fprintf(stderr, "Error in malloc strings\n"); exit(EXIT_FAILURE); }

    // init strings and counters
    for(i=0; i < num_threads; i++){
        strings[i].text = (char *)argv[i+1];
        strings[i].counter = 0;
printf("[%d] %s\n", strings[i].counter, strings[i].text);
    }

    // create file output
    fd = open("output.txt", O_RDWR|O_CREAT|O_TRUNC, 0666);
    if(fd == -1){ fprintf(stderr, "Error in create file\n"); exit(EXIT_FAILURE); }

    /******************** SEMAPHORE ********************/
    sd = semget(IPC_PRIVATE, 1, IPC_CREAT|0666);
    if(sd == -1){ fprintf(stderr, "Error in create semaphore\n"); exit(EXIT_FAILURE); }
    ret = semctl(sd, 0, SETVAL, 1);
    if(ret == -1){ fprintf(stderr, "Error in initialize semaphore %d\n", i); exit(EXIT_FAILURE); }

    /******************** SIGNAL ********************/
    signal(SIGINT, handler);

    /******************** THREADS ********************/
    for(i=0; i < num_threads; i++){
        ret = pthread_create(&tid, NULL, worker, (void*)i);
        if(ret != 0){ fprintf(stderr, "Error in create child %d\n", i); exit(EXIT_FAILURE); }
    }

    /******************** FATHER WORK ********************/
    while(1) pause();

    return EXIT_SUCCESS;
 }
