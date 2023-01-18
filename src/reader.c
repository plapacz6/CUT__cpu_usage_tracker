#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>  //fileno
#include <unistd.h>
// #include <sys/stat.h> //fstat, struct stat
// #include <sys/types.h>
// #include <fcntl.h>  //open
// #include <errno.h>
#include <pthread.h>
#include <threads.h>
#include "ring_buffer.h"
#include "mutexes.h"


/********************  GLOBALS *********************/
//for SIGTERM_handler
char* buff_M;
FILE *proc_stat_file;
char *G_msg_array;
ring_buffer_T *rb_ra;
char *rb_ra_data_table;
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
size_t get_msg_size(size_t n, int get_){
  static size_t msg_size;
  if(get_ == 1) msg_size = n;
  return msg_size;
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
    fprintf(stderr, "%s\n", "can't create temporary file for cat /proc/stat");
  }
  FILE *fin = fopen("/proc/stat", "r");
  if(!fin){
    fclose(fout);
    fprintf(stderr, "%s\n", "can't open /proc/stat");
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

size_t determine_buff_M_size(char *fname){
  FILE *fin = fopen(fname, "r");  
  fseek(fin, 0, SEEK_END);
  size_t end = ftell(fin);
  fclose(fin);
  return end + (unsigned)(end * 0.10);
}

/***************************************************/

char* create_buff_M(size_t buff_M_size){
  char* buff = malloc(buff_M_size);
  if(!buff) {
    printf("%s\n", "can't create bufor_M");
    exit(1);
  }
  return buff;
}
/***************************************************/
void destroy_buff_M(char* buffM){
  if(buffM) free(buffM);
  buffM = NULL;
};

/***************************************************/
int fillin_buff_M_tmpfile(char buff_M[], size_t buff_M_size, char *fname){
  FILE *fin = fopen(fname, "r");
  if(!fin){
    fprintf(stderr, "%s\n", "can't open temporary file for read");
    exit(1);
  }
  size_t readed = fread(buff_M, 1, buff_M_size, fin );
  if(readed < buff_M_size - 1){
    fprintf(stderr, "%s\n", "probably error during reading temporary file");
    fprintf(stderr, "reded [%lu] elements", readed);
  }
  buff_M[buff_M_size - 1] = '\0';
  fclose(fin);
  return 0;
}

/***************************************************/

size_t calc_cpuN(char *buff_M, size_t buf_size){
  char *begin1 = buff_M;
  begin1[buf_size - 1] = '\0';
  char *begin2 = NULL;
  size_t cpuN = 0;
  while( (begin2 = strstr(begin1, "cpu")) != NULL ){    
    cpuN++;
    begin1 = begin2 + 1;
  }
              printf("cpu cores number: %lu\n", cpuN);
  return cpuN;
}

/***************************************************/

/**
 * @brief read from open /proc/fstat to buffer buf_M
 * 
 * @param buf_M 
 * @param buf_size 
 * @param fp 
 */
void read_BM(char *buff_M, size_t buff_M_size, FILE *fp){
  /* read probably whole content from /proc/stat */
  fread(buff_M, 1, buff_M_size - 1, fp);
  buff_M[buff_M_size - 1] = '\0';
            // printf("\n%s\n%s\n%s\n%s\n",
            //   "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"
            //   ,"read_BM:", buff_M,
            //   "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"
            //   );
}

/***************************************************/


/**
 * @brief - parse content of /proc/stat for all cpus
 * 
 * @param buf_M- buffer with content of /proc/stat
 * @param buf_size - size of buff in 1sth param
 * @param cpuN - number of cpu cores, and size of array in 4th param
 * @param p1s - (ptr to 1 set of data) pointer to array of struct proc_stat_1cpu10_T
 * @param ptr_msg - array of size [cpuN][msg_size]
 */

void read_One_set(char buff_M[], size_t buff_M_size, char* msg_array, size_t cpuN, size_t msg_size){
  
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
    strncpy( (msg + (core * msg_size)), begin2, msg_size - 1);    
    *((msg + (core * msg_size)) + msg_size - 1) = '\0';

            //printf("msg:\n%s\n", (msg + (core * msg_size)));

    core++;
    begin1 = begin2 + 1;
  }
}

/***************************************************/

char* create_msg_array(size_t cpu_CorN, size_t msg_size){
  char *msg_a = malloc(cpu_CorN * msg_size);
  if(!msg_a){
    fprintf(stderr, "%s\n", "can't create message array");
    exit(1);
  }
}
/***************************************************/
int destroy_msg_array(char* msg_a){
  if(msg_a) free(msg_a);
  msg_a = NULL;
  G_msg_array = NULL;
}

/***************************************************/

char *create_rb_ra_data_table(size_t size){
  char *data_table = malloc(size);
  if(!data_table){
    fprintf(stderr, "%s\n", "can't create rb_ra_data_table");
    exit(1);
  } 
}
/***************************************************/
int destroy_rb_ra_data_table(char* data_table){
  if(data_table) free(data_table);
  data_table = NULL;
  rb_ra_data_table = NULL;
}

/***************************************************/
int add_msg_to_rb_ra(char *msg_a, size_t msg_a_size, ring_buffer_T *prb){

    //1.pobierz hak
    char *ptr_cell = (char*) rb_get_back_hook(prb);
    //printf("%s\n", "4.4 hook for mgs obtained"); fflush(stdout);
    //2.skopiuj  
    memcpy(ptr_cell, msg_a, msg_a_size);  
    //printf("%s\n", "4.5 message saved in ring_buffer"); fflush(stdout);

  return 0;
}

char* get_msg_from_rb_ra(ring_buffer_T *prb){
  return (char*)rb_get_front_hook(prb);
}
/***************************************************/
void open_proc_stat_file(){
  proc_stat_file = fopen("/proc/stat","r");
  if(!proc_stat_file){
    //thread_exit(1);
    exit(1);
  }  
}
void close_proc_stat_file(){
  if(proc_stat_file != NULL) fclose(proc_stat_file);  
  proc_stat_file = NULL;
}
/***************************************************/

void* reader(void *watchdog_tbl){
  size_t buff_M_size = 0;
  size_t cpu_CorN = 0;
  char *buff_M = NULL;
  
  // printf("\n\n%s\n", "1. reader starting ...");
  // printf("1. buffer_M:---------------\n %s\n____________________________\n", buff_M);


  /* ----------   buffer_M   -----------------*/
  char *restrict fmt = "/var/tmp/CUP__cpu_usage_1stTry_procstat_%d.txt";  
  char fname[strlen(fmt) + 16];
  sprintf(fname, fmt, rand());  

  // printf("\n1.2 tmpfile fname ready: %s\n", fname); fflush(stdout);

  cat_proc_stat_file(fname);

  // printf("\n\n%s\n", "1.3 tmpfile ready");fflush(stdout);

  buff_M_size = determine_buff_M_size(fname);

  // printf("\n\n%s\n", "1.4 buff_M size  - obtained");fflush(stdout); 

  buff_M_size += 2;
  buff_M = create_buff_M(buff_M_size);

  // printf("\n\n%s\n", "1.5 buff_M created");fflush(stdout); 

  fillin_buff_M_tmpfile(&buff_M[0], buff_M_size, fname);
  

  // printf("\n\n%s\n", "1.6 tmpfile reader to buff_M");fflush(stdout); 

  cpu_CorN = calc_cpuN(&buff_M[0], buff_M_size);

  // printf("\n\n%s\n", "1.7 number of cpu cors calculated");fflush(stdout); 

  cpu_cors_N(cpu_CorN, 1);  

  // printf("\n\n%s\n", "1.8 number of cpu cors set");fflush(stdout); 

  if(0 != remove(fname)){
    fprintf(stderr, "%s %s\n", "can't delete temporary file :", fname);
  }
  // printf("\n\n%s\n", "1.9 tmpfile - deleted ");fflush(stdout); 


  // printf("%s\n", "2. buffer_M ready, preparing data table and ring_buff");
  // printf("2. buffer_M:------------\n %s\n____________________________\n", buff_M);

  
  /* ------  data_table + ring_buffer   --------*/


  /* create data table for */
  
  // printf("%s\n", "2.1. setting message array size");

  #define MSG_SIZE (128)
  size_t msg_size = MSG_SIZE;
  get_msg_size(MSG_SIZE, 1);
  #undef MSG_SIZE

  // printf("%s\n", "2.2. creating message array");

  G_msg_array = create_msg_array(cpu_CorN, msg_size);

  /* create ring_buffer */
  rb_ra_data_table = create_rb_ra_data_table(10 * msg_size * cpu_CorN);
  rb_ra = rb_create(rb_ra_data_table, msg_size * cpu_CorN, 10);

  // printf("%s\n", "3. openinig /proc/stat");


  /* ----------   /proc/stat   -----------------*/


  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
  //signal for main to run analyzer thread
  //cnd_signal();

  // printf("%s\n", "4. main loop");
  
  while(1){
  //for(int k = 0; k < 10; k++){
    open_proc_stat_file();  
    // printf("%s\n", "4.1 enter"); fflush(stdout);
    
    read_BM(buff_M, buff_M_size, proc_stat_file);

    // printf("%s\n", "4.2 content /proc/stat readed"); fflush(stdout);

    read_One_set(&buff_M[0], buff_M_size, G_msg_array, cpu_CorN, msg_size);    

    // printf("%s\n", "4.3 messages for analyzer ready"); fflush(stdout);

    /*  podepnij do ring buffera */
    add_msg_to_rb_ra(G_msg_array, msg_size * cpu_CorN, rb_ra);
    
    // /*   test  analyzer odbior*/
    // add_msg_to_rb_ra(G_msg_array, msg_size * cpu_CorN, rb_ra);
    // char *msg_aa = get_msg_from_rb_ra(rb_ra);
    // for(int i = 0; i < cpu_CorN; i++){
    //   printf("\nmsg %d:%s\n", i, (msg_aa + (i * msg_size))); 
    // }

    //3.nanosleep()
    //sleep(1);

    // printf("%s\n", "4.6 after sleep - ready for new /proc/stat read"); fflush(stdout);
    /* -----------------------------*/
    close_proc_stat_file();
  }//while
  printf("%s\n", "5. after main loop"); fflush(stdout); 
  //while(1);

  
  //SIGTERM handler:
  destroy_msg_array(G_msg_array);
  destroy_buff_M(buff_M);
  if(proc_stat_file != NULL) {
    fclose(proc_stat_file);   //SIGTERM
    proc_stat_file = NULL;
  }
  rb_destroy(rb_ra);
  destroy_rb_ra_data_table(rb_ra_data_table);

  pthread_exit(0);
}


/***************************************************/

// int main(){
//   reader(NULL);
//   return 0;
// }