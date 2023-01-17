#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#include "SIGTERM_handler.h"
#include "watchdog.h"
#include "logger.h"
#include "reader.h"
#include "analyzer.h"
//#include "printf.h"
#include "mutexes.h"


//long double *ptr_curr_avr_cpu_usage;  //<<<<<  to analyzer or printer


mtx_t mtx_watchdog;
mtx_t mtx_logger;
mtx_t mtx_reader_analyzer;
mtx_t mtx_analyzer_printer;  

#define NUMBER_OF_MUTEXES (4)

mtx_t* cut_mutexes[NUMBER_OF_MUTEXES] = { 
  &mtx_watchdog,
  &mtx_logger,
  &mtx_reader_analyzer, 
  &mtx_analyzer_printer,
};

void init_mutexes(){
  for(int i = 0; i < NUMBER_OF_MUTEXES; i++){
    if(thrd_success != mtx_init(cut_mutexes[i], mtx_plain)){
      fprintf(stderr, "%s\n", "mutex can't be initialized");
      exit(1);
    }
  }
}

void destroy_mutexes(){  
  for(int i = 0; i < NUMBER_OF_MUTEXES; i++){
    mtx_destroy(cut_mutexes[i]);
  }
}

int main(){
  srand(time(NULL));
  init_mutexes();
  /* registering SIGTERM handler */
  install_SIGTERM_handler();
  
  /* counting number of cpu cors */
  size_t cpu_cores_number = 0;
  //reader::count_number_of_cpu

  /* inicjalizing global objects */
  
  /* ANALYZER-PRINTER*/
  double curr_avr_cpu_usage[cpu_cores_number]; //VLA
  
  
  /* creating 5 threads */
  
  pthread_t pthread_watchdog;
  pthread_t pthread_logger; 
  pthread_t pthread_reader;
  pthread_t pthread_analyzer;
  pthread_t pthread_printer;
  watchdog_table[WATCH_LOGGER].ptr_pthread_id = &pthread_logger;
  watchdog_table[WATCH_LOGGER].ptr_pthread_id = &pthread_reader;
  watchdog_table[WATCH_LOGGER].ptr_pthread_id = &pthread_analyzer;
  watchdog_table[WATCH_LOGGER].ptr_pthread_id = &pthread_printer;

  if(0 != pthread_create(&pthread_watchdog, NULL, watchdog, NULL)){
    printf("%s\n", "pthread_create: watchdog");
    exit(1);
  }
   
  // if(0 != pthread_create(&pthread_logger, NULL, 
  //    logger, &watchdog_table[WATCH_LOGGER].active)){
  // watchdog_table[WATCH_LOGGER].ptr_pthread_id = &pthread_logger;
  // watchdog_table[WATCH_LOGGER].exists = 1; 
  //   printf("%s\n", "pthread_create: LOGGER");
  //   exit(1);
  // }  

  
  if(0 != pthread_create(&pthread_reader, NULL, 
      reader, &watchdog_table[WATCH_READER].active)){
  watchdog_table[WATCH_READER].ptr_pthread_id = &pthread_reader;
  watchdog_table[WATCH_READER].exists = 1; 
    printf("%s\n", "pthread_create: READER");
    exit(1);
  }  

  sleep(1);

 if(0 != pthread_create(&pthread_analyzer, NULL, 
    analyzer, &watchdog_table[WATCH_ANALYZER].active)){
  watchdog_table[WATCH_ANALYZER].ptr_pthread_id = &pthread_analyzer;
  watchdog_table[WATCH_ANALYZER].exists = 1; 
    printf("%s\n", "pthread_create: ANALYZER");
    exit(1);
  }  

//  if(0 != pthread_create(&pthread_printer, NULL, 
//     printer, &watchdog_table[WATCH_PRINTER].active)){
//   watchdog_table[WATCH_PRINTER].ptr_pthread_id = &pthread_printer;
//   watchdog_table[WATCH_PRINTER].exists = 1; 
//     printf("%s\n", "pthread_create: PRINTER");
//     exit(1);
//   }  

  /* ending threads and cleaning */
  // pthread_join(phread_logger, NULL);
  pthread_join(pthread_analyzer, NULL);
  pthread_join(pthread_reader, NULL);
  //pthread_join(pthread_printer, NULL);
  pthread_join(pthread_watchdog, NULL);

  destroy_mutexes();

  // /* this is also in SIGTERM HANDLER*/
  // rb_destroy(ptr_logger_buffer);  
  // if(fstat) fclose(fstat);
  // if(flog) fclose(flog);
  raise(SIGTERM);

  return 0;
}
