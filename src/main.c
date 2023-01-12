#include <signal.h>
#include <string.h>
#include <stdio.h>

#include "SIGTERM_handler.h"
#include "globals.h"
//#include "reader.h"

FILE *fstat = NULL;
FILE *flog = NULL;

long double *ptr_curr_avr_cpu_usage;  
int watchdog_table[4]; 

int main(){

  /* registering SIGTERM handler */
  install_SIGTERM_handler();
  
  /* counting number of cpu cors */
  size_t cpu_cores_number = 0;

  /* inicjalizing global objects */
  double curr_avr_cpu_usage[cpu_cores_number]; //VLA
  /* creating 5 threads */

  /* ending threads and cleaning*/
  if(fstat) fclose(fstat);
  if(flog) fclose(flog);
  return 0;
}
