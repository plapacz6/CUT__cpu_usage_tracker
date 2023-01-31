#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <threads.h>
#include <signal.h>
#include "logger.h"
#include "mutexes.h"


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
  write_log("mutexes", "%s", "mutexes were destoyed");
}

#undef NUMBER_OF_MUTEXES