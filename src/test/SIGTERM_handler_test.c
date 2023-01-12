#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <assert.h>
#include "../SIGTERM_handler.h"

FILE *fstat = NULL;
FILE *flog = NULL;


volatile sig_atomic_t stop_main_loop = 0;
/**
 * @brief Helping SIGTERM singal generator 
 * 
 * @param sec 
 * @return void* 
 */
void *semiwatchdog(void* sec){
  printf("%s %d\n", "Watchdog sleeps for seconds: :", **(int**)sec);
  sleep(**(int**)sec);
  printf("%s\n", "Watchdog sends signal SIGTREM");  
  raise(SIGTERM);  
  printf("%s\n", "this message shoundn't be visible. SIGTERM not raised.");  
  printf("%s\n","SIGTREM handler TEST: FAIL");
  exit(1);
}

/**
 * @brief SIGTERM handler test
 *    
 * @return int 0 if test Pass (handler finished program)
 *             1 if handler didn't work
 */
int main(){
  pthread_t watchdog_thread_id;
  pthread_t* restrict ptr_watchdog_thread_id  = &watchdog_thread_id;
  int sec = 4;
  int *psec = &sec;

  if(0 != pthread_create(ptr_watchdog_thread_id, NULL, semiwatchdog, &psec)){
    fprintf(stderr, "%s\n", "creation thread error");
    exit(1);
  }  

  install_SIGTERM_handler();
  
  FILE *fstat = fopen("/proc/stat", "r");
  printf("%s\n", "main loop start");  
  while(1){
    sleep(1);
    printf("%c", '.');    
    fflush(stdout);
    /* here SIGTERM handler should finish this test program with exit code == 0*/
  }  
  printf("%s\n", "main loop stopped");

  pthread_join(*ptr_watchdog_thread_id, NULL);
  printf("%s\n","SIGTREM handler TEST: FAIL");
  return 1;
}
