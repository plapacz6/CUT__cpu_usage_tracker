#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>  //fileno
#include <unistd.h>
#include <sys/stat.h> //fstat, struct stat
#include <sys/types.h>
#include <fcntl.h>  //open
#include <errno.h>
#include <pthread.h>
#include <threads.h>


/********************  GLOBALS *********************/
char* buff_M;
FILE *proc_stat_file;
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

/**
 * @brief Create a buff_M object
 * 
 * @param ptr_bfMsize - pointer to size_t to fill with size of created buffer
 * @param fp 
 * @return char* NULL if buffer can't be created, size of buffer remain as in last try
 */
char* create_buff_M(size_t *ptr_bfMsize, FILE *fp){
  char* buff;  
  #define ASSUMED_MIN_LINE_LENGTH (264) //10x19+10sp+len(cpu) ~~ 256
  *ptr_bfMsize = ASSUMED_MIN_LINE_LENGTH;  
  
  int i = 0;
  char *ptr_end = NULL;
  buff = malloc(*ptr_bfMsize);  
  if(!buff) {
    return NULL;  
  }  
  char *buff2 = NULL;   //for realloc()
  while(1){
                    printf("%s %d\n", "CREATE BUFFER: try nr:", i++);
    ptr_end = NULL;
    
    fread(buff, 1, *ptr_bfMsize - 2, fp);
    buff[*ptr_bfMsize - 1] = '\0';
    ptr_end = strstr(buff, "intr");
    if(ptr_end != NULL) { 
                    printf("%s\n", "intr  FOUND");
      break;
    }
    *ptr_bfMsize += ASSUMED_MIN_LINE_LENGTH;
    if(*ptr_bfMsize > ASSUMED_MIN_LINE_LENGTH * 16){
      if(buff) {free(buff); buff = NULL;}          
    }    
    buff2 = NULL;
    buff2 = realloc(buff, *ptr_bfMsize);
    if(!buff2) { 
      if(buff) {free(buff); buff = NULL;}  //buff remain valid after not successful realloc      
      return NULL;  
    }    
    buff = buff2;
  }
  /*  try add some extra memory and return  */
  *ptr_bfMsize += (ASSUMED_MIN_LINE_LENGTH * 2);
  buff2 = NULL;
  buff2 = realloc(buff, *ptr_bfMsize);
  if(!buff2) {     //adding extra memory was too much
    if(buff) {free(buff); buff = NULL;}    
    return NULL;
  }  
  return buff;
  #undef ASSUMED_MIN_LINE_LENGTH
}
/***************************************************/
void destroy_buff_M(char** buffM){
  if(buffM) free(*buffM);
  buffM = NULL;
};

/***************************************************/

void read_BM(char *buf_M, size_t buf_size, FILE *fp){
  /* read probably whole content from /proc/stat */
  fread(buff_M, 1, buf_size - 2, fp);
  buff_M[buf_size - 1] = '\0';
            // printf("\n%s\n%s\n%s\n%s\n",
            //   "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"
            //   ,"read_BM:", buff_M,
            //   "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"
            //   );
}

/***************************************************/

size_t calc_cpuN(char *buf_M, size_t buf_size){
  char *begin1 = buff_M;
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
 * @brief - parse content of /proc/stat for all cpus
 * 
 * @param buf_M- buffer with content of /proc/stat
 * @param buf_size - size of buff in 1sth param
 * @param cpuN - number of cpu cores, and size of array in 4th param
 * @param p1s - (ptr to 1 set of data) pointer to array of struct proc_stat_1cpu10_T
 * @param ptr_msg - array of size [cpuN][msg_size]
 */

void read_One_set(char *buf_M, size_t buf_size, char** ptr_msg, size_t cpuN, size_t msg_size){
  
  char *begin1 = buff_M;
  char *begin2 = NULL;  
  begin1 = buff_M;  
  int core = 0;
            printf("\n cpuN: %lu\n", cpuN);
            printf("\n buff_M:\n%s\n", buff_M);
  
  while( (begin2 = strstr(begin1, "core")) != NULL){
            //printf("begin2:\n%s\n", begin2);
    assert(core <= cpuN);    
    if( !(core < cpuN) ) break;    
            //printf("%s", "+"); fflush(stdout);   //TESTY

    //*begin2 = '\0'; //for strncpy
    strncpy( *(ptr_msg + (core * msg_size)), begin2, msg_size - 2);    
    *(*(ptr_msg + (core * msg_size)) + msg_size - 1) = '\0';

            //printf("msg:\n%s\n", ptr_msg[core]);

    core++;
    begin1 = begin2 + 1;
  }
}

/***************************************************/

//extern proc_stat_1cpu10_T *ptr_1set;

/***************************************************/

void* reader(void *watchdog_tbl){
  size_t buff_M_size = 0;
  size_t cpu_CorN = 0;

  proc_stat_file = fopen("/proc/stat","r");
  if(!proc_stat_file){
    //thread_exit(1);
    exit(1);
  }

  if(NULL == (buff_M = create_buff_M(&buff_M_size, proc_stat_file))){
    fprintf(stderr, "%s\n", "can't create working buffer for reader");
    exit(1);
  }  
  fclose(proc_stat_file);

  
  
  //reopoen file after calulating buffor size

  proc_stat_file = fopen("/proc/stat","r");
  if(!proc_stat_file){
    //thread_exit(1);
    exit(1);
  }

  read_BM(buff_M, buff_M_size, proc_stat_file);
  cpu_CorN = calc_cpuN(buff_M, buff_M_size);
  cpu_cors_N(cpu_CorN, 1);
  fclose(proc_stat_file);


  //signal for main to run analyzer thread

  /* create data table for */
  size_t msg_size = 256 + 40 + 1;
  char msg[cpu_CorN][msg_size];       //VLA

  /* create reing buffer */

  //while(1){
  for(int k = 0; k < 1; k++){
    proc_stat_file = fopen("/proc/stat","r");
    if(!proc_stat_file){
    //thread_exit(1);
    exit(1);
  }
    read_BM(buff_M, buff_M_size, proc_stat_file);
    read_One_set(buff_M, buff_M_size, (char**)msg, cpu_CorN, msg_size);    
    /*  podepnij do ring buffera 
    1.pobierz hak
    2.skopiuj  
    3.nanosleep()
    */
    fclose(proc_stat_file);
    sleep(1);
  }//while
  
  //SIGTERM handler:
  destroy_buff_M(&buff_M);
  if(proc_stat_file) fclose(proc_stat_file);   //SIGTERM
  pthread_exit(0);
}


/***************************************************/

int main(){
  reader(NULL);
  return 0;
}