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

/**
 * @brief parse plan text msg into struct describing cpu usage
 * 
 * @param msg 
 * @param data_1cpu 
 */
void parse_msg(char *msg, proc_stat_1cpu10_T *data_1cpu) {

  sscanf(msg, 
        //%SIZE_Hs %Lf %Lf %Lf %Lf %Lf %Lf %Lf %*f %*f %*f",
        "%*s %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf", 
        //header, 
        &data_1cpu->user,
        &data_1cpu->nice,
        &data_1cpu->system,
        &data_1cpu->idle,
        &data_1cpu->iowait,
        &data_1cpu->irq,
        &data_1cpu->softirq,
        &data_1cpu->steal,
        &data_1cpu->guest,
        &data_1cpu->guest_nice
        );    
}

/**
 * @brief calcualte average cpu usage
 * 
 * @param data_1cpu1 
 * @param data_1cpu2 
 * @return long double 
 */
long double calculate_avr_1cpu(proc_stat_1cpu10_T *data_1cpu1, proc_stat_1cpu10_T *data_1cpu2){

  long double     avr = 
    ((( data_1cpu1->user + 
        data_1cpu1->nice +
        data_1cpu1->system + 
        //data_1cpu1->idle 
        data_1cpu1->iowait +
        data_1cpu1->irq +
        data_1cpu1->softirq ) -
        ( data_1cpu2->user + 
        data_1cpu2->nice +
        data_1cpu2->system + 
        //data_1cpu2->idle 
        data_1cpu2->iowait +
        data_1cpu2->irq +
        data_1cpu2->softirq )) /
        (( data_1cpu1->user + 
        data_1cpu1->nice +
        data_1cpu1->system + 
        data_1cpu1->idle +
        data_1cpu1->iowait +
        data_1cpu1->irq +
        data_1cpu1->softirq ) -
        ( data_1cpu2->user + 
        data_1cpu2->nice +
        data_1cpu2->system + 
        data_1cpu2->idle +
        data_1cpu2->iowait +
        data_1cpu2->irq +
        data_1cpu2->softirq )) );
  return avr;
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
      if(*(msg_aa + (i * msg_size)) != '\0'){
        printf("\nmsg %d:%s\n", i, (msg_aa + (i * msg_size)));       
        *(msg_aa + (i * msg_size)) = '\0';
      }
      else {
        printf("\nmsg %d:%s\n", i, "...empty");
      }
    }
    sleep(1);
  }
}


