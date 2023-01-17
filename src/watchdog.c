#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include "watchdog.h"
#include "mutexes.h"

#include <errno.h>

watchdog_entry_T watchdog_table[4];

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

              int ret = pthread_kill(*(watchdog_table[i].ptr_pthread_id), SIGTERM);
              //int ret = pthread_cancel(*(watchdog_table[i].ptr_pthread_id));
              if(0 != ret ){            
                int errsv = errno;
                if(errsv == ESRCH) fprintf(stderr, "%s\n","no such process");
              }
              //thread canceled
              //printf("%s", "K"); fflush(stdout);
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
  



