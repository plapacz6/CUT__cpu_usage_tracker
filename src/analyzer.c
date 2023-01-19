#include <stddef.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

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
        //%SIZE_Hs %Lf %Lf %Lf %Lf %Lf %Lf %Lf %*f %*f %*f",
        "%*s %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %*s", 
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
  return avr * 100;
}

long double *ptr_avr;

// size_t cpu_cors_N(size_t n, int get_);
// size_t get_msg_size(size_t n, int get_);
// char* get_msg_from_rb_ra(ring_buffer_T *prb); 
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
  if(!ptr_avr) free(ptr_avr);
  ptr_avr = NULL;
}
/**************************************************/
void *analyzer(void* watcher_tbl){

  size_t msg_size = get_msg_size(0, 0);
  size_t cpu_CorN = cpu_cors_N(0, 0);
  
  mtx_unlock(&mtx_analyzer_printer);
  ptr_avr = create_avr_array(cpu_CorN);  
  mtx_unlock(&mtx_analyzer_printer);

  //long double moment[2][cpu_CorN];

  proc_stat_1cpu10_T cpu_data[2][cpu_CorN];
  char *msg;
  int pair_ready = 0;  /**< control if there are two set of data for calculate_avr()*/
  int k = 0;
  int m = 1;
  while(1){
    
    char *msg_aa = get_msg_from_rb_ra(rb_ra);
    
    // system("clear");
    // printf("%s\n","average usage cpu:");

    for(int i = 0; i < cpu_CorN; i++){  
      msg = (msg_aa + (i * msg_size));
      #define PAIR_READY (2)
      
      if(msg_aa[0] != '\0'){  //-----------------------------------

        //#define PRINTER_ON

        #ifndef PRINTER_ON
        if(msg[0] != '\0'){        
          parse_msg( msg, &cpu_data[k][i]);

          if(pair_ready != 2)  pair_ready++;
          else  pair_ready = PAIR_READY;

          if(pair_ready == PAIR_READY){
            mtx_lock(&mtx_analyzer_printer);
            ptr_avr[i] = calculate_avr_1cpu(&cpu_data[k][i], &cpu_data[m][i]);
            cnd_signal(&cnd_ap);
            mtx_unlock(&mtx_analyzer_printer);
            //send to printer
            // if(pair_ready == PAIR_READY){
            //   if(i == 0){
            //     printf("\tcpu: %12.2Lf%%\n", ptr_avr[i]);
            //   }
            //   else{
            //     printf("\tcpu%02d: %10.2Lf%%\n", i, ptr_avr[i]);
            //   }
            //   fflush(stdout);
            // }
          }
          k = ++k > 1 ? 0 : 1;
          m = ++m > 1 ? 0 : 1;
          //msg[0] = '\0';          
        } //msg[0] == 0
        else{
          fprintf(stderr, "%s\n", "analyzer: empty msg from ring_buffer");
        }
        
        #else      //DEBUG
        if(msg[0] != '\0'){
          printf("\nmsg %d:%s\n", i, msg);       
          msg[0] = '\0';
        }
        else {
          printf("\nmsg %d:%s\n", i, "...empty");
        }
        #endif
        
      }   //-----------------------------------
      else{  //msg_aa == 0
        fprintf(stderr, "%s\n", "analyzer: empty     msg_aa    from ring_buffer");
      }    //-----------------------------------
    }//for  cpu_CorN
    sleep(1);
    msg_aa[0] = '\0';
  }//while(1)
}
#undef PAIR_READY

