#ifndef READER_H
#define READER_H
#include <stdio.h>

extern FILE *fstat; /**< /proc/stat */

/**
 * @brief /proc/stat
 * entry for one cpu in /proc/stat according to: man 5 proc
 * (first 10 position)
 */
typedef struct proc_stat_1cpu10_TT{  
  char cpuN [10];    // exmpl: "cpu12"
  unsigned long user;
  unsigned long nice;
  unsigned long system;
  unsigned long idle;
  unsigned long iowait;
  unsigned long irq;
  unsigned long softirq;
  unsigned long steal;
  unsigned long guest;
  unsigned long guest_nice;
}proc_stat_1cpu10_T;


#endif // READER_H