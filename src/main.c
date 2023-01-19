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


void release_resouces(void);
void init_mutexes();
void destroy_mutexes();

mtx_t mtx_watchdog;
mtx_t mtx_logger;
mtx_t mtx_reader_analyzer;
mtx_t mtx_analyzer_printer;  
cnd_t cnd_ra;
cnd_t cnd_ap;
cnd_t cnd_log;

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
  cnd_init(&cnd_ra);
  cnd_init(&cnd_ap);
  cnd_init(&cnd_log);
}

void destroy_mutexes(){  
  for(int i = 0; i < NUMBER_OF_MUTEXES; i++){
    mtx_destroy(cut_mutexes[i]);
  }
  cnd_destroy(&cnd_ra);
  cnd_destroy(&cnd_ap);
  cnd_destroy(&cnd_log);  
}


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

  if(0 != pthread_create(&pthread_watchdog, NULL, watchdog, NULL)){
    printf("%s\n", "pthread_create: watchdog");
    exit(1);
  }
   
  // if(0 != pthread_create(&pthread_logger, NULL, 
  //    logger, &watchdog_table[WATCH_LOGGER].active)){
  //   printf("%s\n", "pthread_create: LOGGER");
  //   exit(1);
  // }  
  // register_in_watchdog(WATCH_LOGGER, pthread_logger);

  
  if(0 != pthread_create(&pthread_reader, NULL, 
      reader, &watchdog_table[WATCH_READER].active)){
    printf("%s\n", "pthread_create: READER");
    exit(1);
  }  
  register_in_watchdog(WATCH_READER, pthread_reader);

  sleep(1);
 
 if(0 != pthread_create(&pthread_analyzer, NULL, 
    analyzer, &watchdog_table[WATCH_ANALYZER].active)){
    printf("%s\n", "pthread_create: ANALYZER");
    exit(1);
  }  atexit(release_resouces);
  register_in_watchdog(WATCH_ANALYZER, pthread_analyzer);

  sleep(1);

  if(0 != pthread_create(&pthread_printer, NULL, 
    printer, &watchdog_table[WATCH_PRINTER].active)){
    printf("%s\n", "pthread_create: PRINTER");
    exit(1);
  }  
register_in_watchdog(WATCH_PRINTER, pthread_printer);


  /* ending threads and cleaning */
  // pthread_join(phread_logger, NULL);
  pthread_join(pthread_analyzer, NULL);
  pthread_join(pthread_reader, NULL);
  pthread_join(pthread_printer, NULL);
  pthread_join(pthread_watchdog, NULL);

  release_resouces();
  // /* this is also in SIGTERM HANDLER*/
   raise(SIGTERM);

  return 0;
}

void release_resouces(void){
    
    //logger:
    //if(flog) fclose(flog);           
    //rb_destroy(ptr_logger_buffer);   

    //reader:
    destroy_msg_array(G_msg_array);
    destroy_buff_M(buff_M);
    if(proc_stat_file != NULL) {
      fclose(proc_stat_file);   //SIGTERM
      proc_stat_file = NULL;
    }
    rb_destroy(rb_ra);
    destroy_rb_ra_data_table(rb_ra_data_table);
    destroy_mutexes();
    destroy_avr_array();

    fprintf(stderr, "%s\n", "resources are released");
}