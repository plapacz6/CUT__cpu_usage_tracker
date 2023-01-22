#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include "watchdog.h"
#include "mutexes.h"

#include <errno.h>

watchdog_entry_T watchdog_table[WATCH_TBL_SIZE];

void register_in_watchdog(cell_in_watchdog_table_T idx, pthread_t thrd){
  watchdog_table[idx].ptr_pthread_id = thrd;
  watchdog_table[idx].exists = 1;   
  watchdog_table[idx].active = 1;   
}

void checkin_watchdog(cell_in_watchdog_table_T idx){
  watchdog_table[idx].active = 1;     
}

void cancel_all_pthreads(){
  int i;
  for(i = 0; i < 4; i++){   
    if(watchdog_table[i].exists) {
      int ret = pthread_cancel(watchdog_table[i].ptr_pthread_id);
      if(0 != ret ){                 
        fprintf(stderr, "can't cancel phread: %lu\n", 
            watchdog_table[i].ptr_pthread_id);
      }    
      watchdog_table[i].exists = 0;    
      //DEBUG    
      fprintf(stderr, "watchdog: cancellation of phread: %lu\n", 
      watchdog_table[i].ptr_pthread_id);    
    }
    else{
      //DEBUG
      fprintf(stderr, "watchdog: cancellation of phread THREAD DON'D EXISTS: %lu\n", 
      watchdog_table[i].ptr_pthread_id);    
    }
  }
}

/**
 * @brief watchdog for 4 threads
 * requires the watchdog_table[4] table to be correctly initialized
 * 
 * @return void* 
 */
void* watchdog(){  
  while(1){
    sleep(2);
    if(thrd_success == mtx_trylock(&mtx_watchdog)){
      for(int i = 0; i < 4; i++){      
        if(watchdog_table[i].exists != 0){
          if(! watchdog_table[i].active){                    

              int ret = pthread_cancel(watchdog_table[i].ptr_pthread_id);              
            
              fprintf(stderr, "watchdog: cancellation of pthread: %lu\n", 
                  watchdog_table[i].ptr_pthread_id);

              if(0 != ret ){            
                int errsv = errno;
                if(errsv == ESRCH) fprintf(stderr, "%s\n","watchdog: no such process");
              }

              watchdog_table[i].exists = 0;
          }                    
          else { watchdog_table[i].active = 0; }  
        }         
      }
      mtx_unlock(&mtx_watchdog);
    }//mtx not locked    
  }
  pthread_exit(0);   
} 
  



