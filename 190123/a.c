/*
 *  input:  F pathname, N strings S_1,...,S_N
 *  main:   while 1 read S from stdin, write S in buffer
 *  thread: compare S with S_i
 *  signal: main -> print F
 */

#define SIZE 1024

char buffer[SIZE];

void handler(){
  // print F
}

void child_work(char *string){



  // while(1)
    // wait(new_content_sem)
    // cmp(buffer, S_i)
    // signal(have_read_sem)
}

int main(int argc, char *argv[]) {

  int num_threads;
  int fd;
  int sd;
  pthread_t tid;

  int ret; // aux for return values

  /****************************** USAGE ******************************/
  if(arc < 2){ fprintf(stderr, "[Usage] %s output_file_pathname string_1,...,strings_N\n", argv[0]); exit(EXIT_FAILURE); }

  /****************************** INPUT ******************************/
  // num_threads
  num_threads = argc - 2;
  // file open later only by the father
  // strings in thread spawning

  /****************************** MEMORY ******************************/
  // global buffer

  /****************************** SEMAPHORE ******************************/
  sd = semget(IPC_PRIVATE, 2, IPC_CREAT|0666);
  if(sd == -1){ fprintf(stderr, "Error in creating semaphore\n"); exit(EXIT_FAILURE); }

  ret = semctl(ret, 0, SETVAL, 0);
  if(ret == -1){ fprintf(stderr, "Error in init semaphore\n"); exit(EXIT_FAILURE); }
  ret = semctl(ret, 1, SETVAL, num_threads);
  if(ret == -1){ fprintf(stderr, "Error in init semaphore\n"); exit(EXIT_FAILURE); }

  /****************************** SIGNAL ******************************/
  signal(SIGINT, handler);

  /****************************** THREAD ******************************/
  for(int i=0; i < num_threads; i++){
    ret = pthread_create(&tid, NULL, child_work, argv[i+2]);
    if(ret != 0){ fprintf(stderr, "Error in spawning thread %d\n", i);}
  }

  /****************************** FATHER WORK ******************************/

  fd = open(argv[1], O_RDWR|O_TRUNC|0666);
  if(fd == -1){ fprintf(stderr, "Error in open file\n"); exit(EXIT_FAILURE); }

  while(1){

wait:
    oper.sem_num = 1;
    oper.sem_op = -num_threads;
    oper.sem_flg = 0;
    ret = semop(sd, oper, 1);
    if(ret == -1){
      if(errno == EINTR){ goto wait; }
      else{ fprintf(stderr, "Error in father wait\n"); exit(EXIT_FAILURE); }
    }

    ret = scanf("%s", buffer);
    if(ret == 0){ fprintf(stderr, "Illegale input\n"); exit(EXIT_FAILURE); }

signal:
    oper.sem_num = 0;
    oper.sem_op = 1;
    oper.sem_flg = 0;
    ret = semop(sd, oper, 1);
    if(ret == -1){
      if(errno == EINTR){ goto signal; }
      else{ fprintf(stderr, "Error in father signal\n"); exit(EXIT_FAILURE); }
    }

  }

  return EXIT_SUCCESS;
}
