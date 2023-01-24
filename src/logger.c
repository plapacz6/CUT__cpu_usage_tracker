#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <pthread.h>
#include <threads.h>

#include "mutexes.h"
#include "ring_buffer.h" 
#include "watchdog.h"
#include "logger.h"

volatile sig_atomic_t logger_done = 0;

extern FILE *flog;
extern int logger_buffer_exists;
extern ring_buffer_T *ptr_logger_buffer;

/********************************************************/

FILE *flog = NULL;

void open_log_file(){ 
  char * restrict fname = "/var/tmp/CUT__cpu_usage_tracker.log";
  flog = fopen(fname, "w");
  if(!flog) {
    flog = stderr;
    write_log("logger", "logger can't open log file", fname);
  }
  else {
    setvbuf(flog, 0, _IOLBF, 0); //line buffering ON
  }
}
void close_log_file(){
  if(flog && flog != stderr) {
    fclose(flog); 
    flog = NULL;
  }
}

/********************************************************/
int logger_buffer_exists = 0;
ring_buffer_T *ptr_logger_buffer = NULL;
enum {
   NUMBER_OF_ENTRY = 10,
   MAX_LENGTH_OF_ENTRY  = 256,
   FMT_BUFFOR_SIZE = 256,
   TIMESTAMP_BUF_LEN = 28,
};
char logger_data[NUMBER_OF_ENTRY][MAX_LENGTH_OF_ENTRY];

/* ------------------------------------------------*/

ring_buffer_T *create_logger_buffer(){
  ring_buffer_T *ptr_buffer = rb_create(
      logger_data, 
      MAX_LENGTH_OF_ENTRY, 
      NUMBER_OF_ENTRY);
  if(ptr_buffer == NULL){
    fprintf(stderr, "%s\n", "logger buffer: lack of memory to create");
    return NULL;
  }  
  else {
    logger_buffer_exists = 1;
    return ptr_buffer;
  }  
}

/* ------------------------------------------------*/

void destroy_logger_buffer(){  
    rb_destroy(ptr_logger_buffer);
    ptr_logger_buffer = NULL;
    logger_buffer_exists = 0;  
}
/********************************************************/
void put_msg(char *msg){
  char *ptr4msg = NULL;  
  ptr4msg = (char*) rb_get_back_hook(ptr_logger_buffer),
  assert(ptr4msg != NULL);
  strncpy(ptr4msg, msg, MAX_LENGTH_OF_ENTRY - 1);  
  ptr4msg[MAX_LENGTH_OF_ENTRY -1] = '\0';
}

/* ------------------------------------------------*/

void get_msg(char *msg){  
  strncpy(
    msg,
    (char*)rb_get_front_hook(ptr_logger_buffer),
    MAX_LENGTH_OF_ENTRY - 1
  );
  msg[MAX_LENGTH_OF_ENTRY - 1] = '0';
}

/* ------------------------------------------------*/

void write_log(char who[static 1], char fmt[static 1], ...){

    //generate timestamp
  time_t now = time(NULL);
  struct tm* tm_now = localtime(&now);
  char when[TIMESTAMP_BUF_LEN] = {};
  strftime(when, TIMESTAMP_BUF_LEN - 1, "%Y%m%d_%H:%M:%S", tm_now);

  size_t fmt_len = strlen(fmt);
  char fmt_buff[FMT_BUFFOR_SIZE] = {};
  char msglog[MAX_LENGTH_OF_ENTRY] = {};

  size_t free_place = FMT_BUFFOR_SIZE - 1;
  strncpy(fmt_buff, "%s:%s:", 7);  //7
  free_place -= 6;
  free_place -= TIMESTAMP_BUF_LEN;
  strncat(fmt_buff, fmt, free_place);
  fmt_buff[FMT_BUFFOR_SIZE - free_place] = '\0';    

  va_list arg;
  va_start(arg, fmt);
  bool arg_interpratation = false;
  for(int i = 0; fmt[i] != '\0'; i++){
    if(fmt[i] == '%' && !arg_interpratation){
      switch(fmt[i + 1]){
        case 'l':
          if(fmt[i + 2] == 'u'){  //ul
            i += 2;              
            sprintf(msglog, fmt_buff, who, when, va_arg(arg, unsigned long));              
            arg_interpratation = true;
          }
          else {    //l
            break;
          }            
          break;
        case 'd':
          i++;
          sprintf(msglog, fmt_buff, who, when, va_arg(arg, int));              
          arg_interpratation = true;
          break;
        case 's':
          i++;
          sprintf(msglog, fmt_buff, who, when, va_arg(arg, char*));              
          arg_interpratation = true;
          break;
      }
    }
  }
  if(!arg_interpratation){
    sprintf(msglog, "%s:%s :%s", who, when, fmt);
    arg_interpratation = true;
  }
  va_end(arg);

  if(logger_buffer_exists){
    put_msg(msglog);
  }
  else {
    fprintf(stderr,"\n%s\n", msglog); fflush(stderr);
  }
}
/* ------------------------------------------------*/
/**
 * @brief if logger phread is canceled function write_log switch on printing on stderr
 * 
 * @param arg 
 */
void logger_clean_up(void *arg){
  //mtx_lock(&mtx_logger);
  destroy_logger_buffer();  //logger_buffer_exist ==> 0
  close_log_file();
  //mtx_unlock(&mtx_logger);
  logger_buffer_exists = 0;
  fprintf(stderr, "logger: resouces released, switching on stderr");
}
/********************************************************/
void* logger(void *arg){  
  pthread_cleanup_push(logger_clean_up, NULL);
  ptr_logger_buffer = create_logger_buffer();
  open_log_file();
  char msglog[MAX_LENGTH_OF_ENTRY]; // + TIMESTAMP_BUF_LEN + FMT_BUFFOR_SIZE];
  if(logger_buffer_exists){
    while(!logger_done){

      
        get_msg(msglog);
        fprintf(flog, "%s\n", msglog);
        //fprintf(stderr, "logger: emul.flog:__%s\n", msglog);
      
      
      mtx_lock(&mtx_watchdog);
      checkin_watchdog(WATCH_LOGGER);
      mtx_unlock(&mtx_watchdog);

    }//while
  }
  else{  //only check in with watchdog 
    while(1){
      mtx_lock(&mtx_watchdog);
      checkin_watchdog(WATCH_LOGGER);
      mtx_unlock(&mtx_watchdog);
      sleep(1);
    }
  } //if buffer exits       
  
  pthread_exit(0);
  pthread_cleanup_pop(1);  
}