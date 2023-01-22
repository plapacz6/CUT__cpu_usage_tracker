#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <threads.h>

#include "../watchdog.h"

/* ---------------------------------------------- */
mtx_t mtx_watchdog;
/* ---------------------------------------------- */

/******    dummy  SIGTERM hander *******************/

void SIGTERM_handler(int signum){
  
  sigset_t signal_mask;
  sigset_t old_signal_mask;
  sigfillset(&signal_mask);
  sigprocmask(SIG_BLOCK, &signal_mask, &old_signal_mask);
  
  if(signum == SIGTERM || signum == SIGINT){        
    mtx_destroy(&mtx_watchdog);
    cancel_all_pthreads();
    fprintf(stderr, "%s\n", "resources released");
    quick_exit(0);
  }

  sigprocmask(SIG_SETMASK, &old_signal_mask, NULL);
}

void install_SIGTERM_handler(){
  struct sigaction action;
  memset(&action, 0, sizeof(struct sigaction));
  action.sa_handler  = SIGTERM_handler;
  if(0 != sigaction(SIGTERM, &action, NULL)){
    fprintf(stderr, "%s\n", "SIGTERM handler registration error.");
    exit(1);
  }
  // if(0 != sigaction(SIGINT, &action, NULL)){
  //   fprintf(stderr, "%s\n", "SIGINT handler registration error.");
  //   exit(1);
  // }  
}

/**************************************************/


/* ---------------------------------------------- */
void test_worker_cleanup(void* a){
  fprintf(stderr, "%s\n", "watchdog test PASS"); fflush(stderr); //!!! w/o  \n
}
/* ---------------------------------------------- */
void *test_worker1(void* arg){
  pthread_cleanup_push(test_worker_cleanup, NULL);

  for(int i = 0; i < 4; i++){
                printf("%s","w"); fflush(stdout);
    mtx_lock(&mtx_watchdog);
    checkin_watchdog(WATCH_PRINTER);
    mtx_unlock(&mtx_watchdog);
    sleep(1);
  }                
  while(1){}  
  pthread_cleanup_pop(1);
  pthread_exit(NULL);
}
/* ---------------------------------------------- */
void *test_worker2(void* arg){  
  for(int i = 0;; i++){
    mtx_lock(&mtx_watchdog);
    checkin_watchdog(WATCH_ANALYZER);
    mtx_unlock(&mtx_watchdog);
                printf("%s","x"); fflush(stdout);
    sleep(1);                
    if(i > 5){            
      printf("%s","X"); fflush(stdout);
      raise(SIGTERM);
      sleep(5);
      printf("%s","V"); fflush(stdout);
    }
  }
  pthread_exit(NULL);
}
/* ---------------------------------------------- */
void *test_worker3(void* arg){
  int i = 0;
  while(1){        
    mtx_lock(&mtx_watchdog);
    checkin_watchdog(WATCH_READER);
    mtx_unlock(&mtx_watchdog);
                printf("%s","o"); fflush(stdout);    
    sleep(8);
    i++;
    if(i > 2) while(1);
  }
  pthread_exit(NULL);
}
/* ---------------------------------------------- */
void *test_worker4(void* arg){
  int i = 0;
  while(1){    
    mtx_lock(&mtx_watchdog);
    checkin_watchdog(WATCH_LOGGER);
    mtx_unlock(&mtx_watchdog);
                printf("%s","#"); fflush(stdout);    
    sleep(10);
    i++;
    if(i > 2) while(1);
  }
  pthread_exit(NULL);
}

/* ---------------------------------------------- */

int main(){  
  install_SIGTERM_handler();
  
  if(thrd_success != mtx_init(&mtx_watchdog, mtx_plain)){
    fprintf(stderr, "%s\n", "mtx_init: mtx_watchdog  error");
    exit(1);
  }

  pthread_t watchdog_id; 
  if(0 != pthread_create(&watchdog_id, NULL, watchdog, NULL)){
    fprintf(stderr, "%s\n", "pthread_create: watchdog  error");
    exit(1);
  }
  
  pthread_t test_worker_id_1;
  pthread_t test_worker_id_2;
  pthread_t test_worker_id_3;
  pthread_t test_worker_id_4;

  mtx_lock(&mtx_watchdog);
    
    if(0 != pthread_create(&test_worker_id_1, NULL, test_worker1,NULL)){
      fprintf(stderr, "%s\n", "pthread_create: watchdog  error");
      exit(1);
    }     
    register_in_watchdog(WATCH_PRINTER, test_worker_id_1);


    if(0 != pthread_create(&test_worker_id_2, NULL, test_worker2, NULL)){
      fprintf(stderr, "%s\n", "pthread_create: watchdog  error");
      exit(1);
    }         
    register_in_watchdog(WATCH_ANALYZER, test_worker_id_2);


    if(0 != pthread_create(&test_worker_id_3, NULL, test_worker3, NULL)){
      fprintf(stderr, "%s\n", "pthread_create: watchdog  error");
      exit(1);
    }     
    register_in_watchdog(WATCH_READER, test_worker_id_3);


    if(0 != pthread_create(&test_worker_id_4, NULL, test_worker4, NULL)){
      fprintf(stderr, "%s\n", "pthread_create: watchdog  error");
      exit(1);
    }     
    register_in_watchdog(WATCH_LOGGER, test_worker_id_4);

/*  jakis tam komentarz */

/*****/

          /*****/
  mtx_unlock(&mtx_watchdog);
  pthread_t cup_phreads[4] = {
    test_worker_id_1, 
    test_worker_id_2,
    test_worker_id_3,
    test_worker_id_4,      
  };
  sleep(15);
  //raise(SIGTERM);
  pthread_cancel(watchdog_id);
  pthread_join(watchdog_id, NULL);
  printf("%s","M"); fflush(stdout);
  fflush(stdout);
  
  return 0;
}