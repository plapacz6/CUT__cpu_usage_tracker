#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <pthread.h>
#include <threads.h>
#include <assert.h>

#include "../reader.h"

int cat_proc_stat_file(char *fname);


int test_cat_proc_stat_file(){
  char * restrict fmt = "/var/tmp/CUP__cpu_usage_test_reader1_%d.txt";  
  char fname[strlen(fmt) + 16];
  sprintf(fname, fmt, rand());  
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

  if(0 != remove(fname)){
    fprintf(stderr, "%s %s\n", "can't delete temporary file :", fname);
  }
  return 0;
}
#undef BUFF_SIZE

int main(){
  srand(time(NULL));
  int ret = 0;
  ret = test_cat_proc_stat_file();
  
  return 
}