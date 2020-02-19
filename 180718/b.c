/*
 *  input:  n pathname
 *  main:   stdin -> buffer
 *  thread: write buffer in myfile
 *  signal:
 *    - thread: print myfile
 *    - main: ignore
 *
 */

/*
 * SEM_A for new content available [0 = no content, 1 = new content]
 * SEM_B for all the children has read [0, ..., num_threads have read]
 */


#define SIZE 128

char buffer[SIZE];
int sd;

void handler(){
  // print my file
}

void child_work(void *pathname){

  signal(SIGINT, handler);
  //while(1) wait(new_content)
    // write on my file
    // signal(i_have_read)
}

int main(int argc, char *argv[]) {

  int num_threads;

  int ret; // aux for return value
  pthread_t tid;

  /****************************** USAGE ******************************/
  if(argc < 2) { printf("[Usage] %s file_pathname1 ... file_pathnameN\n", argv[0]); exit(EXIT_FAILURE); }

  /****************************** INPUT ******************************/
  num_threads = argc - 1;
  // pathnames

  /****************************** MEMORY ******************************/


  /****************************** SEMAPHORE ******************************/
  sd = semget(IPC_PRIVATE, 2, IPC_CREAT|0666);
  if(sd == -1) { printf("Error in semget\n"); exit(EXIT_FAILURE); }

  ret = semctl(sd, 0, SETVAL, 0);
  if(ret == -1) { printf("Error in semctl sem[0]\n"); exit(EXIT_FAILURE); }
  ret = semctl(sd, 1, SETVAL, num_threads);
  if(ret == -1) { printf("Error in semctl sem[1]\n"); exit(EXIT_FAILURE); }

  /****************************** THREAD ******************************/
  for(int i = 0; i < num_threads; i++){
    ret = pthread_create(tid, NULL, child_work, argv[i+1]);
    if(ret == -1) { printf("Error creating thread %d\n", i); exit(EXIT_FAILURE); }
  }

  /****************************** SIGNAL ******************************/
  signal(SIGINT, SIG_IGN);

  /****************************** FATHER WORK ******************************/
  //wait(all_have_read)
wait:
  oper.sem_num = 1;
  oper.sem_op = -num_threads;
  oper.sem_flg = 0;
  ret = semop(sd, &oper, 1);
  if(ret == -1) {
    if(errno == EINTR) goto wait;
    else { printf("Error in wait father\n"); exit(EXIT_FAILURE); }
  }

  //while(1) scanf(buffer)
  while(1){
    while( scanf("%s\n", buffer) == EOF){

    }
  }

  //signal(new_content)
signal:
  oper.sem_num = 0;
  oper.sem_op = 1;
  oper.sem_flg = 0;
  ret = semop(sd, &oper, 1);
  if(ret == -1) {
    if(errno == EINTR) goto signal;
    else { printf("Error in signal father\n"); exit(EXIT_FAILURE); }
  }

  return EXIT_SUCCESS;

}
