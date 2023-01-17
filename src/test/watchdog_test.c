#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

#include "../watchdog.h"
#include "../mutexes.h"
#include "../SIGTERM_handler.h"


void test_worker_cleanup(void* a){
  printf("%s\n", "watchdog test PASS"); fflush(stdout); //!!! w/o  \n
}

void *test_worker1(void* ptr_rejestr){
  pthread_cleanup_push(test_worker_cleanup, NULL);

  for(int i = 0; i < 4; i++){
                printf("%s","w"); fflush(stdout);
    mtx_lock(&mtx_watchdog);
    *((int*)ptr_rejestr) = 1;
    mtx_unlock(&mtx_watchdog);
    sleep(1);
  }                
  while(1){}  
  pthread_cleanup_pop(1);
  pthread_exit(NULL);
}
void *test_worker2(void* ptr_rejestr){
  int i = 0;
  while(1){
    mtx_lock(&mtx_watchdog);
    *((int*)ptr_rejestr) = 1;
    mtx_unlock(&mtx_watchdog);
                printf("%s","x"); fflush(stdout);
    sleep(1);
    i++;
    if(i > 10){
      raise(SIGTERM);
    }
  }
  pthread_exit(NULL);
}
void *test_worker3(void* ptr_rejestr){
  int i = 0;
  while(1){        
    mtx_lock(&mtx_watchdog);
    *((int*)ptr_rejestr) = 1;
    mtx_unlock(&mtx_watchdog);
                printf("%s","o"); fflush(stdout);    
    sleep(8);
    i++;
    if(i > 2) while(1);
  }
  pthread_exit(NULL);
}
void *test_worker4(void* ptr_rejestr){
  int i = 0;
  while(1){    
    mtx_lock(&mtx_watchdog);
    *((int*)ptr_rejestr) = 1;
    mtx_unlock(&mtx_watchdog);
                printf("%s","#"); fflush(stdout);    
    sleep(10);
    i++;
    if(i > 2) while(1);
  }
  pthread_exit(NULL);
}


mtx_t mtx_watchdog;

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
    
    if(0 != pthread_create(&test_worker_id_1, NULL, test_worker1, &watchdog_table[0].active)){
      fprintf(stderr, "%s\n", "pthread_create: watchdog  error");
      exit(1);
    }     
    watchdog_table[0].ptr_pthread_id = &test_worker_id_1; 
    watchdog_table[0].exists = 1;

    if(0 != pthread_create(&test_worker_id_2, NULL, test_worker2, &watchdog_table[1].active)){
      fprintf(stderr, "%s\n", "pthread_create: watchdog  error");
      exit(1);
    }         
    watchdog_table[1].ptr_pthread_id = &test_worker_id_2;        
    watchdog_table[1].exists = 1;

    if(0 != pthread_create(&test_worker_id_3, NULL, test_worker3, &watchdog_table[2].active)){
      fprintf(stderr, "%s\n", "pthread_create: watchdog  error");
      exit(1);
    }     
    watchdog_table[2].ptr_pthread_id = &test_worker_id_3;        
    watchdog_table[2].exists = 1;

    if(0 != pthread_create(&test_worker_id_4, NULL, test_worker4, &watchdog_table[3].active)){
      fprintf(stderr, "%s\n", "pthread_create: watchdog  error");
      exit(1);
    }     
    watchdog_table[3].ptr_pthread_id = &test_worker_id_4;        
    watchdog_table[3].exists = 1;
          
  mtx_unlock(&mtx_watchdog);
  

  return 0;
}