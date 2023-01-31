#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <threads.h>

#include "logger.h"
#include "reader.h"
#include "analyzer.h"
#include "printer.h"
#include "watchdog.h"
#include "mutexes.h"

#include "SIGTERM_handler.h"

void SIGTERM_handler(int signum){
  
  sigset_t signal_mask;
  sigset_t old_signal_mask;
  sigfillset(&signal_mask);
  sigprocmask(SIG_BLOCK, &signal_mask, &old_signal_mask);
  
  if(signum == SIGTERM || signum == SIGINT){
    
    printer_done = 1;
    
    fprintf(stderr, "%s\n", "SIGTERM handler: pthreads loops broken");    
  }

  sigprocmask(SIG_SETMASK, &old_signal_mask, NULL);
}

void install_SIGTERM_handler(){
  struct sigaction action;
  memset(&action, 0, sizeof(struct sigaction));
  action.sa_handler  = SIGTERM_handler;
  if(0 != sigaction(SIGTERM, &action, NULL)){
    fprintf(stderr, "%s\n", "SIGTERM handler registration error.");
    exit(1);
  }
  if(0 != sigaction(SIGINT, &action, NULL)){
    fprintf(stderr, "%s\n", "SIGTERM handler registration error.");
    exit(1);
  }  
}

