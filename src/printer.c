#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
//#include <time.h>
//#include <string.h>
#include <unistd.h>
#include <threads.h>
#include <pthread.h>
#include <assert.h>

#include "printer.h"
#include "reader.h"  //cpu_N
#include "analyzer.h"
#include "watchdog.h"
//#include "logger.h"
#include "mutexes.h"

void *printer(void* watcher_tbl){
  
  assert(ptr_avr != NULL);

  size_t msg_size = get_msg_size(0, 0);
  size_t cpu_CorN = cpu_cors_N(0, 0);
  
  long double avr[cpu_CorN];
  
  while(1){
    mtx_lock(&mtx_analyzer_printer);    
    cnd_wait(&cnd_ap, &mtx_analyzer_printer);
    assert(ptr_avr != NULL);

    system("clear");
    printf("%s\n","printer: \naverage usage cpu:");

    for(int i = 0; i < cpu_CorN; i++){  
                        
              if(i == 0){
                printf("\tcpu: %12.2Lf%%\n", ptr_avr[i]);
              }
              else{
                printf("\tcpu%02d: %10.2Lf%%\n", i, ptr_avr[i]);
              }
              fflush(stdout);            
    }//for cpu_CorN
    
    mtx_unlock(&mtx_analyzer_printer);
    
    //menu ctl+c = end        
    // printf("\n\n\n");
    // printf("\nctl + c = exit");
    sleep(1);
  }//while(1)
}
