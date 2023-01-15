#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "logger.h"
#include "reader.h"
#include "ring_buffer.h"


#include "SIGTERM_handler.h"
void SIGTERM_handler(int signum){
  if(signum == SIGTERM){
    if(fstat) fclose(fstat);           //reader
    //if(flog) fclose(flog);           //logger
    //rb_destroy(ptr_logger_buffer);   //logger
    exit(0);
  }
}

void install_SIGTERM_handler(){
  struct sigaction action;
  memset(&action, 0, sizeof(struct sigaction));
  action.sa_handler  = SIGTERM_handler;
  if(0 != sigaction(SIGTERM, &action, NULL)){
    fprintf(stderr, "%s\n", "SIGTERM handler registration error.");
    exit(1);
  }
}
