#ifndef ANALYZER_H
#define ANALYZER_H

#include <stddef.h>
#include <pthread.h>

#include "watchdog.h"
#include "ring_buffer.h"
#include "reader.h"
#include "mutexes.h"

/**
 * @brief /proc/stat
 * entry for one cpu in /proc/stat according to: man 5 proc
 * (first 10 position)
 */
typedef struct proc_stat_1cpu10_TT{  
  //char cpuN [10];    // exmpl: "cpu12"
  long double user;
  long double nice;
  long double system;
  long double idle;
  long double iowait;
  long double irq;
  long double softirq;
  long double steal;
  long double guest;
  long double guest_nice;
} proc_stat_1cpu10_T;

/**
 * @brief pointer to array of calculated average usage for each cpu core
 * 
 * Array is created by analyzer thred and pointer is then initialized.
 * Printer thred wait on conditional variable for signal from analyzer, that 
 * array is created and pointer to it initialzed, and some reasonable average
 * values are stored in that array.
 */
extern long double *ptr_avr;


void destroy_avr_array();

/**
 * @brief main function of analyzer thread
 * 
 * @param watcher_tbl 
 * @return void* 
 */
void *analyzer(void* arg);


#endif // ANALYZER_H