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
#include "../mutexes.h"

#include "../logger.h"
#include "../reader.h"
#include "../analyzer.h"
#include "../printer.h"
#include "../watchdog.h"

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
  
  // FILE *fstat = fopen("/proc/stat", "r");
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

/*************  stub functions ****************************/

//logger
void write_log(char who[static 1], char fmt[static 1], ...){
  char tmp1 = *who;
  char tmp2 = *fmt;
  tmp2 = tmp1;
  tmp1 = tmp2;  
}

void* logger(void *){return NULL;}

void reader_release_resources(void* ){}
size_t cpu_cors_N(size_t , int) {return 0;}
size_t get_size_msg1core(size_t , int) {return 0;}
char* get_msg_from_rb_ra(void) {return NULL;}
void* reader(void *) {return NULL;}

//extern 
long double *ptr_avr = NULL;
void analyzer_release_resources(void*) {}
void *analyzer(void* ){return NULL;}

void *printer(void*){return NULL;}

// extern 
watchdog_entry_T watchdog_table[4]; 
void register_in_watchdog(cell_in_watchdog_table_T , pthread_t ){}
void checkin_watchdog(cell_in_watchdog_table_T ){}
void cancel_all_pthreads(){}
void* watchdog(){return NULL;}