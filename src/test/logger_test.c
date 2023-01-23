#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <threads.h>
#include <assert.h>

#include "../logger.h"
#include "../ring_buffer.h" 
#include "../mutexes.h"




FILE *flog;
int logger_buffer_exists;
typedef int cell_in_watchdog_table_T;
ring_buffer_T *ptr_logger_buffer;
void checkin_watchdog(cell_in_watchdog_table_T idx){  

}

int main(){  
  init_mutexes();

  pthread_t pthread_logger;
  pthread_create(&pthread_logger, NULL, logger, NULL);

  char buffer[10];    
  for(int i = 0; i < 3; i++){
    sprintf(buffer, "%d", i);
    write_log("main", "nr", buffer);
    sleep(1);
  }

  pthread_cancel(pthread_logger);
  pthread_join(pthread_logger, NULL);
  destroy_mutexes();
  

  write_log("main", "nr", "4. on stderr");
  fflush(stderr);

  FILE *flog = fopen("/var/tmp/CUT__cpu_usage_tracker.log", "r");
  char buffor[100];
  fscanf(flog, "%s", buffor);  
  if(flog) fclose(flog);
  fprintf(stderr, "%s\n", buffor); fflush(stderr);
  
  assert(0 == strncmp(buffor, "main", 4));

  return 0;
}

/**
 * czy masak singalow jest zachowywania w wywolanych fn?
 * 
 * czy raise jest funckja, i blokuje na callerze syngal
 * ktory on wywoluje ?
 * 
 */