#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

//#include "globals.h"
#include "mutexes.h"
#include "logger.h"
#include "ring_buffer.h" 
#include "watchdog.h"


ring_buffer_T *ptr_logger_buffer;
FILE *flog = NULL;

void logger_clean_up(void *a){
  mtx_unlock(&mtx_watchdog);
  mtx_unlock(&mtx_logger);  
  if(flog) fclose(flog);
}

void* logger(void *ptr_watchdog_place_4_logger){
  pthread_cleanup_push(logger_clean_up, NULL);

  int* watchdog_rejestr = (int*)ptr_watchdog_place_4_logger;
  //ring_buffer_T   
  #define NUMBER_OF_ENTRY (10)
  #define MAX_LENGTH_OF_ENTRY (256)
  char logger_data[NUMBER_OF_ENTRY][MAX_LENGTH_OF_ENTRY];

  /* ptr_logger_buffer exits on the heap, so it don't die when thread is canceled*/
  ptr_logger_buffer = rb_create(
      logger_data, 
      MAX_LENGTH_OF_ENTRY, 
      NUMBER_OF_ENTRY);
  if(ptr_logger_buffer == NULL){
    fprintf(stderr, "%s\n", "logger buffer: lack of memory to create");
    //exit(1)
    raise(SIGTERM); 
  }
  
  //static char const fmt[] = "/var/tmp/CUT__cpu_usage_tracker_%d_.
  char *fmt = "/var/tmp/CUT__cpu_usage_tracker_%d_.log";
  char fname[16 + sizeof fmt]; //VL    
  //flog = fopen("~/CUT__cpu_usage_tracker.log", "a");
  snprintf(fname, sizeof fname, fmt, rand());

  flog = fopen(fname, "w");  
  if(flog){  
    setvbuf(flog, 0, _IOLBF, 0); //line buffering ON

    char **ptr_msg = NULL;  
    while(1){
      mtx_lock(&mtx_logger);
      ptr_msg = rb_get_front_hook(ptr_logger_buffer);        
      assert(ptr_msg != NULL);
      if((*ptr_msg)[0] != '\0'){
        fprintf(flog, "%s\n", *ptr_msg);
      }    
      ptr_msg[0] = '\0';
      mtx_unlock(&mtx_logger);
      
      mtx_lock(&mtx_watchdog);
      watchdog_rejestr[WATCH_LOGGER] = 1;
      mtx_unlock(&mtx_watchdog);
      
      sleep(1);   
    }//while
  }
  else{
    fprintf(stderr, "%s\n", "logger don't work.  can't open log file.");      
  }     
  pthread_cleanup_pop(1);
  pthread_exit(0);
}

/**
 * @brief send message to logger
 * 
 * @param msg message - NULL terminating string
 */
void log_msg(char *msg){
  char **ptr4msg = NULL;
  ptr4msg = rb_get_back_hook(ptr_logger_buffer);
  assert(ptr4msg != NULL);
  strncpy(*ptr4msg, msg, MAX_LENGTH_OF_ENTRY - 2);
  *ptr4msg[MAX_LENGTH_OF_ENTRY -1] = '\0';
}