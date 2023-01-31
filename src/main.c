#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include <stdlib.h>

#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

#include "SIGTERM_handler.h"
#include "watchdog.h"
#include "logger.h"
#include "reader.h"
#include "analyzer.h"
#include "printer.h"
#include "mutexes.h"
//#include "ring_buffer.h"



void release_resouces(void){
  write_log("main:clean","release resources");
}

int printer_debug_on = 0;

int main(int argc, char** argv){  
  srand(time(NULL));
  init_mutexes();

  if(argc == 2 && argv[1][0] == 'd'){
    printer_debug_on = 1;
  }
  
  /* not used */
  if(0 != at_quick_exit(release_resouces)){
    fprintf(stderr, "%s\n", "registration of at_exit() fail");
  }
  if(0 != atexit(release_resouces)){
    fprintf(stderr, "%s\n", "registration of at_exit() fail");
  }  
  /* registering SIGTERM handler */
  install_SIGTERM_handler();
   
  /* creating 5 threads */
  pthread_t pthread_watchdog;
  pthread_t pthread_logger; 
  pthread_t pthread_reader;
  pthread_t pthread_analyzer;
  pthread_t pthread_printer;

  if(0 != pthread_create(&pthread_watchdog, NULL, watchdog, NULL)){
    printf("%s\n", "pthread_create: watchdog");
    exit(1);
  }
   
   
  if(0 != pthread_create(&pthread_logger, NULL, logger, NULL)){
    printf("%s\n", "pthread_create: LOGGER");
    exit(1);
  }  
  register_in_watchdog(WATCH_LOGGER, pthread_logger);
  

  if(0 != pthread_create(&pthread_reader, NULL, reader, NULL)){
    printf("%s\n", "pthread_create: READER");
    exit(1);
  }  
  register_in_watchdog(WATCH_READER, pthread_reader);


  sleep(1);
 

  if(0 != pthread_create(&pthread_analyzer, NULL, analyzer, NULL)){
    printf("%s\n", "pthread_create: ANALYZER");
    exit(1);
  }
  register_in_watchdog(WATCH_ANALYZER, pthread_analyzer);


  if(0 != pthread_create(&pthread_printer, NULL, printer, NULL)){
    printf("%s\n", "pthread_create: PRINTER");
    exit(1);
  }  
  register_in_watchdog(WATCH_PRINTER, pthread_printer);


  pthread_join(pthread_printer, NULL); 
  analyzer_done = 1;

  pthread_join(pthread_analyzer, NULL);
  reader_done = 1;
  pthread_join(pthread_reader, NULL); 
  logger_done = 1;
  pthread_join(pthread_logger, NULL);
  watchdog_done = 1;

  pthread_join(pthread_watchdog, NULL);

  write_log("main", "after joining all phreads");

  destroy_mutexes();
  write_log("main", "after destorying mutexes");
  return 0;
}
