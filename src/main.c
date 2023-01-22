#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>

#include <stdlib.h>
#include <stdlib.h>

#include <pthread.h>
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

/*
struct timespec now;
timespec_get(&now, TIME_UTC);
now.tv_sec += 1;
cnd_timedwait(cnd, mtx, &now);
*/

int main(){  
  srand(time(NULL));
  init_mutexes();
  
  /* can't link to at_exit()  and  at_quick_exit() */
  // if(0 != at_quick_exit(release_resouces)){
  //   fprintf(stderr, "%s\n", "registration of at_exit() fail");
  // }
  // if(0 != at_exit(release_resouces)){
  //   fprintf(stderr, "%s\n", "registration of at_exit() fail");
  // }  
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
    raise(SIGTERM);
  }
   
   
  if(0 != pthread_create(&pthread_logger, NULL, logger, NULL)){
    printf("%s\n", "pthread_create: LOGGER");
    raise(SIGTERM);
  }  
  register_in_watchdog(WATCH_LOGGER, pthread_logger);
  

  if(0 != pthread_create(&pthread_reader, NULL, reader, NULL)){
    printf("%s\n", "pthread_create: READER");
    raise(SIGTERM);
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

  pthread_t cup_phreads[5] = {
    pthread_logger, 
    pthread_reader,
    pthread_analyzer,
    pthread_printer,  
    pthread_watchdog 
  };
  for(int i = 0; i < 5; i ++){
    pthread_join(cup_phreads[i], NULL);
    // if(0 != pthread_detach(cup_phreads[i])) {    
    //   fprintf(stderr, "%s\n", "can't detached pthred");
    //   raise(SIGTERM);
    // }
  }
  return 0;
}
