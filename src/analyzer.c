#include <stddef.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include "logger.h"
#include "analyzer.h"
#include "watchdog.h"
#include "ring_buffer.h"
#include "reader.h"
#include "mutexes.h"
/**************************************************/

/**
 * @brief parse plan text msg into struct describing cpu usage
 * 
 * @param msg 
 * @param data_1cpu 
 */
void parse_msg(char *msg, proc_stat_1cpu10_T *data_1cpu) {

  sscanf(msg,         
        "%*s %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %*s",         
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
long double calculate_avr_1cpu(
    proc_stat_1cpu10_T *data_1cpu1, 
    proc_stat_1cpu10_T *data_1cpu2){

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
  return avr * 100;  // %
}

long double *ptr_avr;

/**************************************************/
long double *create_avr_array(size_t cpu_CorN){
  ptr_avr = calloc(sizeof(long double), cpu_CorN);
  if(!ptr_avr){
    fprintf(stderr, "%s\n", "can't create 'average' array");
    exit(1);
  }  
  return ptr_avr;
}
void destroy_avr_array(){
  if(ptr_avr) free(ptr_avr);
  ptr_avr = NULL;
}
/**************************************************/
void *analyzer(void* arg){

  size_t size_msg1core = get_size_msg1core(0, 0); /**< size of raw message describing 1 cpu core*/
  size_t cpu_CorN = cpu_cors_N(0, 0);
  
  mtx_unlock(&mtx_analyzer_printer);
  ptr_avr = create_avr_array(cpu_CorN);  
  mtx_unlock(&mtx_analyzer_printer);  

  char *msg_all_cors; /**< raw message describing whole cpu*/
  char *msg;    /**< part of raw message describing whole cpu, concern 1 core*/

  #define PAIR_READY (2)
  int pair_ready = 0;  /**< control if there are two set of data for calculate_avr()*/
  int k = 0;
  int m = 1;
  proc_stat_1cpu10_T cpu_data[2][cpu_CorN]; /**< array of struct with values for 1 core*/

  while(1){  //main loop
    
    msg_all_cors = get_msg_from_rb_ra();  
    
    //#define DEBUG_PRINT_ON        
    #ifdef DEBUG_PRINT_ON       //DEBUG
    //printf("%s\n", "analyzer:");
    #endif

    for(int i = 0; i < cpu_CorN; i++){  
      
      msg = (msg_all_cors + (i * size_msg1core));
            
      if(msg_all_cors[0] != '\0'){  //-----------------------------------

        if(msg[0] != '\0'){        
          parse_msg( msg, &cpu_data[k][i]);

          if(pair_ready != 2)  pair_ready++;
          else  pair_ready = PAIR_READY;

          if(pair_ready == PAIR_READY){
            mtx_lock(&mtx_analyzer_printer);
            ptr_avr[i] = calculate_avr_1cpu(&cpu_data[k][i], &cpu_data[m][i]);
            cnd_signal(&cnd_ap);
            mtx_unlock(&mtx_analyzer_printer);           
          }
          k = ++k > 1 ? 0 : 1;
          m = ++m > 1 ? 0 : 1;          
        } //msg[0] == 0
        else{
          write_log("analyzer", "%s", "analyzer: empty msg from ring_buffer");
        }
                      
        #ifdef DEBUG_PRINT_ON     //DEBUG
        if(msg[0] != '\0'){
          printf("\nmsg %d:%s\n", i, msg);       
          msg[0] = '\0';}
        else {
          printf("\nmsg %d:%s\n", i, "...empty");}
        #endif
        
      } //-----------------------------------------------------------------
      else{  //msg_all_cors == 0
        write_log("analyzer", "%s", "analyzer: empty     msg_all_cors    from ring_buffer");
      } //------------------------------------------------------------------
    }//for  cpu_CorN

    mtx_lock(&mtx_watchdog);
    checkin_watchdog(WATCH_ANALYZER);
    mtx_unlock(&mtx_watchdog);

    //sleep(1);
    msg_all_cors[0] = '\0';   //raw message used
  }//while(1)
}
#undef PAIR_READY

