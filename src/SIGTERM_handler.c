#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "globals.h"


#include "SIGTERM_handler.h"
void SIGTERM_handler(int signum){
  if(signum == SIGTERM){
    if(fstat) fclose(fstat);
    if(flog) fclose(flog);
    #ifdef TEST_ON
    printf("%s\n", "SIGTERM_handler TEST PASS");
    #endif //TEST_ON    
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
