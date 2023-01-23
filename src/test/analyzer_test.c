#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include "../reader.h"
#include "../ring_buffer.h"
#include "../SIGTERM_handler.h"
#include "../analyzer.h"
#include "../mutexes.h"

/**********************   dummy function for SIGTERM hander *******************/
//logger
void close_log_file(){
}
void destroy_logger_buffer(){
}
//watchdog
void checkin_watchdog(cell_in_watchdog_table_T idx){  
}
void cancel_all_pthreads(){
}
// //reader
// char* buff_M;
// FILE *proc_stat_file;
// char *G_msg_array;
// ring_buffer_T *rb_ra;
// char *rb_ra_data_table;
// void destroy_buff_M(char* buffM){
// }
// int destroy_msg_array(char* msg_a){
// }
// int destroy_rb_ra_data_table(char* data_table){
// }
// void close_proc_stat_file(){
// }
// //analzer
// void destroy_avr_array(){
// }
/***********************************************************************/

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



int main(){
  init_mutexes();
  int ret = 0;
  ret = test_parse_msg();
  ret = test_cacluate_avr_1cpu();

  destroy_mutexes();
  return 0;
}