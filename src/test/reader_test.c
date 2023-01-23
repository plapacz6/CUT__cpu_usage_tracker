#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <pthread.h>
#include <threads.h>
#include <unistd.h>
#include <assert.h>
#include "../logger.h"
#include "../reader.h"



#define PR_TEST_FUNC_NAME(X) printf(\
"\n-----------------------------------------"\
"\nTEST FUNCTION: %s\n", #X );

/************* dummy function for test environemnt *******************/
typedef int cell_in_watchdog_table_T;
void checkin_watchdog(cell_in_watchdog_table_T idx){  
}
// void write_log(char *who, char *msg, char* arg){
//   fprintf(stderr, "%s %s %s\n", who, msg, arg);
// }

/***** internal reader function not exported in reader.h : ***********/
int cat_proc_stat_file(char *fname);
size_t determine_buff_M_size(char *fname);
int fillin_buff_M_tmpfile(char* buff_M, size_t buff_M_size, char *fname);
void* reader(void *watchdog_tbl);

/***************************************************/
int test_cat_proc_stat_file(char *fname){
  PR_TEST_FUNC_NAME(cat_proc_stat_file);
  if(0 != cat_proc_stat_file(fname)){
    fprintf(stderr, "%s\n", " cat_proc_stat_file : fail");
    return -1;
  }

 #define BUFF_SIZE (4096) 
  char buff_for_file[BUFF_SIZE] = {};

  FILE *fin0 = fopen(fname, "r");
  if(!fin0) {
    printf("%s\n", "can't open file created by tested function");
    return -1;
  }
  fread(buff_for_file, 1, BUFF_SIZE - 2, fin0);
  buff_for_file[BUFF_SIZE - 1] = '\0';
  fclose(fin0);

  printf("\n%s\n", "=============================");
  int i = 0;
  for(i = 0; i < BUFF_SIZE; i++){
    fputc(buff_for_file[i], stdout);    
  }
  assert(i != 0);  //file is not empty
  printf("\n%s\n", "=============================");  


  char* found_pattern1 = NULL;
  found_pattern1 = strstr(buff_for_file, "cpu");
  char *found_pattern2 = NULL;
  found_pattern2 = strstr(buff_for_file, "intr");
  assert(found_pattern1 != NULL && found_pattern2 != NULL);
  //found at least one beginning of interesting part
  // &&
  //found pattern after interesting part of file

  return 0;
}
#undef BUFF_SIZE

/***************************************************/

int test_determine_buff_M_size(char *fname){
  PR_TEST_FUNC_NAME(determine_buff_M_size);
  size_t size = determine_buff_M_size(fname);
  printf("%s   file size: %lu", fname, size);
  assert(size > 43); //one cpu at least
  return 0;
}

/***************************************************/

int test_fillin_buff_M_tmpfile(char *fname){
  PR_TEST_FUNC_NAME(fillin_buff_M_tmpfile);
  char buff[100] = {};
  size_t size = 100;
  fillin_buff_M_tmpfile(buff, size, fname);
  for(int i = 0; i < 10; i++){
    if(buff[i] == 0) return 1;
  }
  return 0;
} 

/***************************************************/

int main(){
  srand(time(NULL));
  int ret = 0;

  char * restrict fmt = "/var/tmp/CUP__cpu_usage_test_reader1_%d.txt";  
  char fname[strlen(fmt) + 16];
  sprintf(fname, fmt, rand());  
  
  ret = test_cat_proc_stat_file(fname);
  ret = test_determine_buff_M_size(fname);
  ret = test_fillin_buff_M_tmpfile(fname);
  if(0 != remove(fname)){
    fprintf(stderr, "%s %s\n", "can't delete temporary file :", fname);
  }

  //checking if reader thread will start work
  pthread_t pthread_reader;
  pthread_create(&pthread_reader, NULL, reader, NULL);
  sleep(2);
  pthread_cancel(pthread_reader);
  pthread_join(pthread_reader, NULL);

  return ret;
}