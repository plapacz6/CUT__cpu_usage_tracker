#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
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

void bar_create(char* bar, long double val);

void *printer(void* arg){
    
  size_t msg_size = get_size_msg1core(0, 0);
  size_t cpu_CorN = cpu_cors_N(0, 0);    
  
  char bar[81];

  while(1){
    mtx_lock(&mtx_analyzer_printer);    
    cnd_wait(&cnd_ap, &mtx_analyzer_printer);
    assert(ptr_avr != NULL);

    //system("clear");
    printf("%s\n","printer:");  //DEBUG  ""
    printf("%s\n","average usage cpu:");

    for(int i = 0; i < cpu_CorN; i++){  
              bar_create(bar, ptr_avr[i]);
              if(i == 0){
                printf("\tcpu: %12.2Lf%%  %s\n", ptr_avr[i], bar);
              }
              else{
                printf("\tcpu%02d: %10.2Lf%%  %s\n", i, ptr_avr[i], bar);
              }
              fflush(stdout);            
    }//for cpu_CorN
    
    mtx_unlock(&mtx_analyzer_printer);
    
    //menu ctl+c = end        
    printf("%s", "\n\n\n");
    printf("%s\n", "ctl + c = exit");
    
    mtx_lock(&mtx_watchdog);
    checkin_watchdog(WATCH_PRINTER);
    mtx_unlock(&mtx_watchdog);

    //sleep(1);
  }//while(1)
}

void bar_create(char* bar, long double val){
  int full_len = 54-8; //46
  int val_len = (full_len * val)/100;
  int i;
  for(i = 0; i < val_len; i++){
    bar[i] = '!'; //'='; //'|'; //'*'; //'I'; //'X';
  }
  bar[i] = '\0';
}