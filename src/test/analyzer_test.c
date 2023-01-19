#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include "../reader.h"
#include "../ring_buffer.h"
#include "../SIGTERM_handler.h"
#include "../analyzer.h"
#include "../mutexes.h"

void parse_msg(char *msg, proc_stat_1cpu10_T *data_1cpu);
long double calculate_avr_1cpu(proc_stat_1cpu10_T *data_1cpu1, proc_stat_1cpu10_T *data_1cpu2);

int test_parse_msg(){
  char *msg = "cpu 1 2 3 4 5 6 7 8 9 10";
  proc_stat_1cpu10_T cpu_data;
  parse_msg( msg, &cpu_data );

  assert(cpu_data.user == 1);
  assert(cpu_data.nice == 2);
  assert(cpu_data.system == 3);
  assert(cpu_data.idle == 4);
  assert(cpu_data.iowait == 5);
  assert(cpu_data.irq == 6);
  assert(cpu_data.softirq == 7);
  assert(cpu_data.steal == 8);
  assert(cpu_data.guest == 9);
  assert(cpu_data.guest_nice == 10);
  return 0;
}


int test_cacluate_avr_1cpu(){
  char *msg1 = "cpu 1 2 3 4 5 6 7 8 9 10";
  proc_stat_1cpu10_T cpu_data1;
  parse_msg( msg1, &cpu_data1 );
  
  char *msg2 = "cpu 9 3 4 5 6 7 8 9 10 11";
  proc_stat_1cpu10_T cpu_data2;
  parse_msg( msg2, &cpu_data2 );

  long double avr = calculate_avr_1cpu(&cpu_data1, &cpu_data2);
  // 1+2+3+5+6+7   == 24 
  // 1+2+3+4+5+6+7 == 28 
  // 9+3+4+6+7+8   == 37
  // 9+3+4+5+6+7+8 == 42 
  // (24-37)/(28-42) == 0.9285714285714286
  int avr_1 = (int)(avr*1000);  
  int avr_2 = (int)(   ((24.0L-37.0L)/(28.0L-42.0L))  * 1000  );
  printf("%s\n","");
  printf("avr_1: %d %Lf\n", avr_1, avr);
  printf("avr_2: %d %Lf\n", avr_2, 1000 * 0.9285714285714286L);  
  assert(avr_1 == avr_2);

  return 0;
}



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
}


int main(){
  init_mutexes();
  int ret = 0;
  ret = test_parse_msg();
  ret = test_cacluate_avr_1cpu();

  destroy_mutexes();
  return 0;
}