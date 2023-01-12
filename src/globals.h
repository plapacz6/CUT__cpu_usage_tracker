#ifndef GLOBALS_H
#define GLOBALS_H
#include <stdio.h>
#include "buffer_ra.h"
#include "buffer_log.h"

extern FILE *fstat; /**< /proc/stat */
extern FILE *flog;  /**<  logger file */

//extern buffer_ra_t buff_ra; /**< buffer for communication reader-->analyzer */
//extern buffer_log_t buff_log; /**< buffer for logger */
extern long double *ptr_curr_avr_cpu_usage;  /**< table of avr value produced by analyzer for printer*/
extern int watchdog_table[4]; /**< register for monitoring thread activity */


#endif // GLOBALS_H