#ifndef WATCHER_H
#define WATCHER_H
#include <pthread.h>
#include <signal.h>

typedef struct watchdog_entry_TT {
  int exists;
  int active;
  pthread_t ptr_pthread_id;  
} watchdog_entry_T;

typedef enum cell_in_watchdog_table_TT {
  WATCH_PRINTER  = 0,
  WATCH_ANALYZER = 1,
  WATCH_READER   = 2,
  WATCH_LOGGER   = 3,
  WATCH_TBL_SIZE = 4
} cell_in_watchdog_table_T;

extern watchdog_entry_T watchdog_table[4]; /**< register for monitoring thread activity */

/**
 * @brief register pthread in watchdog table of activity threads
 *
 * each interested pthread call this function only ones, so there shouldn't be deadlocks
 *
 * @param idx 
 * @param thrd 
 */
void register_in_watchdog(cell_in_watchdog_table_T idx, pthread_t thrd);
/**
 * @brief writes pthread to attendance list
 * Each interested phread call this function only in one place, 
 * and exits only one thread each kind,
 * so shouldn't be deadlocks
 * @param idx 
 */
void checkin_watchdog(cell_in_watchdog_table_T idx);

/**helping functioni for SITGERM handleer*/
volatile sig_atomic_t watchdog_done;
void cancel_all_pthreads();

void* watchdog();

#endif// WATCHER_H
