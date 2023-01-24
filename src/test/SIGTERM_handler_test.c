/*************************  TESTS ************************/
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
#include "../ring_buffer.h"
//#include "../mutexes.h"






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
  sleep(15);
  printf("%s\n","SIGTREM handler TEST: FAIL");
  exit(1);
}

//logger
void write_log(char who[static 1], char fmt[static 1], ...){
}
void close_log_file(){
  fprintf(stderr, "%s\n", "logger file closed");
}
void destroy_logger_buffer(){
  fprintf(stderr, "%s\n", "logger_buffer destroyed");
}
void logger_clean_up(void *arg){  
  fprintf(stderr, "%s\n", "logger: clean up");
}


//reader
volatile sig_atomic_t reader_done = 0;
volatile sig_atomic_t analyzer_done = 0;
volatile sig_atomic_t logger_done = 0;
volatile sig_atomic_t printer_done = 0;
volatile sig_atomic_t watchdog_done = 0;

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
  while(!printer_done){
    sleep(1);
    printf("%c", '.');    
    fflush(stdout);    
  }  
  printf("%s\n", "main loop stopped, SIGTERM_handler is working");
  pthread_cancel(*ptr_watchdog_thread_id); //cancel before phread exit(1)
  pthread_join(*ptr_watchdog_thread_id, NULL);
  printf("%s\n","SIGTREM handler TEST: PASS");
  return 0;
}
