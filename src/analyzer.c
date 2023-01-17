#include <stddef.h>
#include <pthread.h>
#include <unistd.h>

#include "analyzer.h"
#include "watchdog.h"
#include "ring_buffer.h"
#include "reader.h"
#include "mutexes.h"
/**************************************************/
/*
typedef struct proc_stat_1cpu10_TT{  
  unsigned long user;
  unsigned long nice;
  unsigned long system;
  unsigned long idle;
  unsigned long iowait;
  unsigned long irq;
  unsigned long softirq;
  unsigned long steal;
  unsigned long guest;
  unsigned long guest_nice;
} proc_stat_1cpu10_T;
*/


/**
 * @brief read entry for one core
 * 
 */
void read_entry_one_core(FILE* fp, long double* a[], char* head){
fscanf(fp,
      "%*s %Lf %Lf %Lf %Lf %Lf %Lf %Lf %*f %*f %*f",      
      a[0],a[1],a[2],a[3],a[4],a[5],a[6]
    );    
}


long double calculate_avr_1cpu(proc_stat_1cpu10_T data_1cpu){

  // long double** loadavg;
  // long double avr[3];
  // proc_stat_1cpu10_T *p1s = NULL;
  // proc_stat_1cpu10_T *p2s = NULL;
  // for(int i = 0; i < 8; i++){

  //   // if(p2s == NULL){
  //   //   p2s = p1s;
  //   // }
   
  //   for(int cpu = 0; cpu < 3; cpu++) {
  //     loadavg[i][cpu] = 
  //     ((( p1s[cpu].user + 
  //         p1s[cpu].nice +
  //         p1s[cpu].system + 
  //         //p1s[cpu].idle 
  //         p1s[cpu].iowait +
  //         p1s[cpu].irq +
  //         p1s[cpu].softirq ) -
  //         ( p2s[cpu].user + 
  //         p2s[cpu].nice +
  //         p2s[cpu].system + 
  //         //p2s[cpu].idle 
  //         p2s[cpu].iowait +
  //         p2s[cpu].irq +
  //         p2s[cpu].softirq )) /
  //         (( p1s[cpu].user + 
  //         p1s[cpu].nice +
  //         p1s[cpu].system + 
  //         p1s[cpu].idle +
  //         p1s[cpu].iowait +
  //         p1s[cpu].irq +
  //         p1s[cpu].softirq ) -
  //         ( p2s[cpu].user + 
  //         p2s[cpu].nice +
  //         p2s[cpu].system + 
  //         p2s[cpu].idle +
  //         p2s[cpu].iowait +
  //         p2s[cpu].irq +
  //         p2s[cpu].softirq )) );
  //   }//for 1zestaw oblicza

  //   p2s++;
  //   p1s++;

  // }//for 10 zestawowo
  return 0;
}

// size_t cpu_cors_N(size_t n, int get_);
// size_t get_msg_size(size_t n, int get_);
// char* get_msg_from_rb_ra(ring_buffer_T *prb); 
/**************************************************/
void *analyzer(void* watcher_tbl){
  
  size_t msg_size = get_msg_size(0, 0);
  size_t cpu_CorN = cpu_cors_N(0, 0);
  while(1){
  /*   test  analyzer odbior*/    
    char *msg_aa = get_msg_from_rb_ra(rb_ra);
    for(int i = 0; i < cpu_CorN; i++){
      if(*(msg_aa + (i * msg_size)) != NULL){
        printf("\nmsg %d:%s\n", i, (msg_aa + (i * msg_size))); \        
        *(msg_aa + (i * msg_size)) = '\0';
      }
      else {
        printf("\nmsg %d:%s\n", i, "...empty");
      }
    }
    sleep(1);
  }
}


