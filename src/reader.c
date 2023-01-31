#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h> 
#include <unistd.h>
#include <signal.h>

#include <pthread.h>
#include <threads.h>
#include "ring_buffer.h"
#include "logger.h"
#include "watchdog.h"
#include "mutexes.h"
#include "reader.h"

volatile sig_atomic_t reader_done = 0;
/********************  GLOBALS *********************/
//pointers of static scope
char* buff_M = NULL;
char *array4entry4allcore = NULL;
FILE *proc_stat_file = NULL;

char *rb_ra_data_table = NULL;
ring_buffer_T *rb_ra = NULL;

/***************************************************/

/**
 * @brief get number of cpu cors, or set it
 * 
 * @param n - number of cors to set
 * @param get_     1 - number of cors is set on n,
 *                 0 or not 1 - n is ignored
 * @return size_t - number of cpu cors
 */
size_t cpu_cors_N(size_t n, int get_){
  static size_t cpu_cors_number;
  if(get_ == 1) cpu_cors_number = n; 
  return cpu_cors_number;
}
/***************************************************/
/**
 * get length of one raw entry in /proc/stat desrcibin all cores, but
 * not containing information folowing "intr" string
 * 
*/
size_t get_size_msg1core(size_t n, int get_){
  static size_t size_msg1core;
  if(get_ == 1) size_msg1core = n;
  return size_msg1core;
}
/***************************************************/

/**
 * @brief similiar to cat /proc/stat > file
 * 
 * @param fname   name file to create
 * @return int    0  - success
 *                -1 - error
 */
int cat_proc_stat_file(char *fname){
  FILE* fout = fopen(fname, "w");
  if(!fout) {
    write_log("reader", "%s", "can't create temporary file for cat /proc/stat");
  }
  FILE *fin = fopen("/proc/stat", "r");
  if(!fin){
    fclose(fout);
    write_log("reader", "%s", "can't open /proc/stat");
    exit(1);
  }
  char c;
  while(!feof(fin)){
    c = fgetc(fin);
    fputc(c, fout);
  }
  fclose(fout);
  fclose(fin);
  return 0; //ok
}

/***************************************************/

/**
 * @brief reads the contents of a temporary file 
 * to determine the buffer size for such contents
 * 
 * During the operation, it opens and closes the file fname. 
 * The fname file should exist.
 * 
 * @param fname 
 * @return size_t 
 */
size_t determine_buff_M_size(char *fname){
  FILE *fin = fopen(fname, "r");  
  fseek(fin, 0, SEEK_END);
  size_t end = ftell(fin);
  fclose(fin);
  return end + (unsigned)(end * 0.10);
}

/***************************************************/

/**
 * @brief Create a char buffer on the heap
 * 
 * @param buff_M_size 
 * @return char*   - pointer to created buffer
 */
char* create_buff_M(size_t buff_M_size){  
  char* buff = (char*)malloc(buff_M_size);
  if(!buff) {
    printf("%s\n", "can't create bufor_M");
    exit(1);
  }
  memset(buff, 0, buff_M_size);
  return buff;
}
/***************************************************/

/**
 * @brief function for STERM_handler, to destroy buffor alocated on heap
 * 
 * destroyed buffor is pointed by static scope pointer buff_M
 */
void destroy_buff_M(void){
  if(buff_M) free(buff_M);
  buff_M = NULL;
}

/***************************************************/

/**
 * @brief fill in buff_M (of size buff_M_size) with content of file fname
 * 
 * Filled in buffer can be further processed by other function.
 * During operation function open and close file fname.
 * 
 * @param buff_M 
 * @param buff_M_size 
 * @param fname 
 * @return int  0 - ok, 
 *              in case of error terminates the entire program by exit(1)
 */
int fillin_buff_M_tmpfile(char buff_M[], size_t buff_M_size, char *fname){
  FILE *fin = fopen(fname, "r");
  if(!fin){
    write_log("reader", "%s", "can't open temporary file for read");    
    exit(1);
  }
  size_t readed = fread(buff_M, 1, buff_M_size, fin );
  if(readed < buff_M_size - 1){
    write_log("reader", "%s", "probably error during reading temporary file");
    write_log("reader", "reded [%lu] elements", readed);        
  }
  buff_M[buff_M_size - 1] = '\0';
  fclose(fin);
  return 0;
}

/***************************************************/

/**
 * @brief calculate number of cpu cors
 * 
 * @param buff_M - char buffer filled in with content of /proc/stat
 * @param buf_size - size of buffer
 * @return size_t  - number of cpu cors
 */
size_t calc_cpuN(char *buff_M, size_t buf_size){  
  char *begin1 = buff_M;
  begin1[buf_size - 1] = '\0';
  char *begin2 = NULL;
  size_t cpuN = 0;
  while( (begin2 = strstr(begin1, "cpu")) != NULL ){    
    cpuN++;
    begin1 = begin2 + 1;
  }
              write_log("reader:calc_cpuN", "cpu cores number: %lu\n", cpuN);
  return cpuN;
}

/***************************************************/

/** 
 * @brief read from open /proc/fstat to buffer buf_M
 * 
 * Function don't open and close file.
 * 
 * @param buf_M 
 * @param buf_size 
 * @param fp 
 */
void read_BM(char *buff_M, size_t buff_M_size, FILE *fp){
  /* read probably whole content from /proc/stat */
  fread(buff_M, 1, buff_M_size - 1, fp);
  buff_M[buff_M_size - 1] = '\0';
}

/***************************************************/

/**
 * @brief - parse content of /proc/stat for all cpus
 * 
 * @param buf_M- buffer with content of /proc/stat
 * @param buf_size - size of buff in 1sth param
 * @param cpuN - number of cpu cores, and size of array in 4th param
 * @param p1s - (ptr to 1 set of data) pointer to array of struct proc_stat_1cpu10_T
 * @param ptr_msg - array of size [cpuN][size_msg1core]
 */

void read_One_set(char buff_M[], size_t buff_M_size, char* msg_array, size_t cpuN, size_t size_msg1core){
  
  char *begin1 = buff_M;
  char *begin2 = NULL;  
  begin1 = &buff_M[0];  
  int core = 0;
            // printf("\n cpuN: %lu\n", cpuN);
            // buff_M[buff_M_size - 1] = '\0';
            // printf("\n buff_M:\n%s\n", &buff_M[0]);
  
  while( (begin2 = strstr(begin1, "cpu")) != NULL){
            //printf("begin2:\n%s\n", begin2);
    assert(core <= cpuN);    
    if( !(core < cpuN) ) break;    
            //printf("%s", "+"); fflush(stdout);   //TESTY

    //*begin2 = '\0'; //for strncpy
    // printf("\n%%%%%%%%%%%%%%%%\n: ", "");
    // printf("sizeof(char**): %lu\n", sizeof(char**));
    // printf("sizeof(char*):  %lu\n", sizeof(char*));
    // printf("sizeof(char):   %lu\n", sizeof(char));
    // printf("\n%%%%%%%%%%%%%%%%\n: ", "");
    char* msg = (char*) msg_array;
    strncpy( (msg + (core * size_msg1core)), begin2, size_msg1core - 1);    
    *((msg + (core * size_msg1core)) + size_msg1core - 1) = '\0';

            //printf("msg:\n%s\n", (msg + (core * size_msg1core)));

    core++;
    begin1 = begin2 + 1;
  }
}

/***************************************************/

/**
 * @brief Create a msg_a array for raw entry concerning all cpu cors
 * 
 * All other information from raw entry readed from /proc/stat was cutted off
 * 
 * @param cpu_CorN 
 * @param size_msg1core 
 * @return char* 
 */
char* create_array_4_all_core(size_t cpu_CorN, size_t size_msg1core){
  char *msg_a = (char*)malloc(cpu_CorN * size_msg1core);
  if(!msg_a){
    write_log("reader", "%s", "can't create message array");
    exit(1);
  }
  return msg_a;
}

/**
 * @brief function for SITTERM_handler destroy char array pointed by of static scope pointer array4entry4allcore
 * 
 */
void destroy_array_4_all_core(void) {
  if(array4entry4allcore) free(array4entry4allcore);
  array4entry4allcore = NULL;  
}

/***************************************************/

/**
 * @brief Create a ring buffer to communication between reader and analyzer
 * 
 * Sets of static scope pointer ra_rb and rb_ra_data_table, which
 * are use during free allocated resouces.
 * 
 */
void create_rb_ra(void){  
  rb_ra_data_table = NULL;
  char *rb_ra_data_table = (char*)malloc(10 * get_size_msg1core(0,0) * cpu_cors_N(0,0));
  if(!rb_ra_data_table){
    write_log("reader", "%s", "can't create rb_ra_data_table");
    raise(SIGTERM);
    //exit(1);
  }     
  rb_ra = rb_create(rb_ra_data_table, get_size_msg1core(0,0) * cpu_cors_N(0,0), 10);  
}

/**
 * @brief helping function for SITTERM_handler 
 * for destroying ring buffer reader-analyzer and release its resources.
 * 
 */
void destroy_rb_ra(void){    
  if(rb_ra) rb_destroy(rb_ra);
    rb_ra = NULL;
  if(rb_ra_data_table) free(rb_ra_data_table);  
    rb_ra_data_table = NULL;    
}
// void destroy_rb_ra_data_table(void){
//     if(rb_ra_data_table) free(rb_ra_data_table);  
//     rb_ra_data_table = NULL;    
// }
/* ----------------------------------------------- */

/**
 * @brief add packet of messages describing all cpu cores
 * 
 * @param msg_all_cpu 
 * @param msg_all_cpu_size 
 * @param prb 
 * @return int 
 */
int add_msg_to_rb_ra(char *msg_all_cpu, size_t msg_all_cpu_size){
      //1.get a hook
    char *ptr_cell = (char*) rb_get_back_hook(rb_ra);    
      //2.copy 
    memcpy(ptr_cell, msg_all_cpu, msg_all_cpu_size);      
  return 0;
}
/**
 * @brief Get the msg_all_cpu from ring buffer pointeb by rb_ra
 * 
 * rb_ra is static scope pointer to ring buffer (not visible beyond reader.c)
 * 
 * @return char* 
 */
char* get_msg_from_rb_ra(void){
  return (char*)rb_get_front_hook(rb_ra);
}

/***************************************************/

/**
 * @brief open file /proc/stat
 * 
 * on failure to open, terminates the entire program by exit(1)
 * 
 */
void open_proc_stat_file(void){
  proc_stat_file = fopen("/proc/stat","r");
  if(!proc_stat_file){    
    exit(1);
  }  
}

/**
 * @brief close file /proc/stat
 * 
 */
void close_proc_stat_file(void){
  if(proc_stat_file != NULL) fclose(proc_stat_file);  
  proc_stat_file = NULL;
}

/***************************************************/

/**
 * @brief helping function for SIGTERM_handler 
 * releases resources aquired by reader pthread
 * 
 */
void reader_release_resources(void* arg){
  destroy_rb_ra();  
  //destroy_rb_ra_data_table(); 

  destroy_array_4_all_core();
  destroy_buff_M();    
  close_proc_stat_file();

  write_log("reader", "%s", "resource resleased");
}
/***************************************************/

void* reader(void *arg){
  pthread_cleanup_push(reader_release_resources, NULL);
  
  size_t buff_M_size = 0;
  size_t cpu_CorN = 0;
  
  /**----- create buffor for content of file /proc/stat  ---- * 
   * 1.crete name for temporary file
   * 2.cat file /proc/stat
   * 3.measure size of temporary created file
   * 4.create main buffor for reads from /proc/stat
   * 5.read in new content of /proc/stat
   * 6.besed on that temporary file count number of cpu cores
   * 7.delete temporary file
   * 
   */
    //1
  char *restrict fmt = "/var/tmp/CUP__cpu_usage_1stTry_procstat_%d.txt";  
  char fname[strlen(fmt) + 16];
  sprintf(fname, fmt, rand());  
    //2
  cat_proc_stat_file(fname);
    //3
  buff_M_size = determine_buff_M_size(fname);
    //4
  buff_M_size += 2;
  buff_M = create_buff_M(buff_M_size);
    //5
  fillin_buff_M_tmpfile(&buff_M[0], buff_M_size, fname);
    //6
  cpu_CorN = calc_cpuN(&buff_M[0], buff_M_size);
  cpu_cors_N(cpu_CorN, 1);  
    //7
  if(0 != remove(fname)){
    write_log("reader", "can't delete temporary file %s:", fname);
  }
  
  /** --- prepare ring_buffer for communication with analyzer ----*
   * 1. set size of 1 raw entry for 1 cpu core 
   * 2. create array for raw entry for all cpu core 
   *    (table not containig informacji not concerning cpu cors)
   * 2. create ring buffer and his data table
   */
    //1.
  #define MSG_SIZE (128)
  size_t size_msg1core = MSG_SIZE;
  get_size_msg1core(MSG_SIZE, 1);
  #undef MSG_SIZE
    //2.
  array4entry4allcore = create_array_4_all_core(cpu_CorN, size_msg1core);
    //3.
  create_rb_ra();
  

  /** --- main loop : ---------------------------
   * 1. opening /proc/stat
   * 2. reading form that file to main buffer (buff_M)
   * 3. processing raw entry being in main buffer (buff_M)
   * 4. sending processed packet of messeges to analyzer 
   *    (through ring buffer rb_ra)
   * 5. check in in watcher list
   * 6. close /proc/stat
   * 7. sleep one second
   */
    
  while(!reader_done){
      //1.      
    open_proc_stat_file();  
      //2.
    read_BM(buff_M, buff_M_size, proc_stat_file);
      //3.
    read_One_set(&buff_M[0], buff_M_size, array4entry4allcore, cpu_CorN, size_msg1core);    
      //4.    
    add_msg_to_rb_ra(array4entry4allcore, size_msg1core * cpu_CorN);
      //5.    
    mtx_lock(&mtx_watchdog);
    checkin_watchdog(WATCH_READER);
    mtx_unlock(&mtx_watchdog);
      //6.
    close_proc_stat_file();
      //7.
    sleep(1);
    
  }//while
  write_log("reader", "%s", "main loop done");
  pthread_exit(0);
  pthread_cleanup_pop(1);    
}
/***************************************************/
