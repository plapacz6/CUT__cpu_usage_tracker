#ifndef WATCHER_H
#define WATCHER_H

typedef struct watchdog_entry_TT {
  int exists;
  int active;
  pthread_t *ptr_pthread_id;  
} watchdog_entry_T;

typedef enum cell_in_watchdog_table_TT {
  WATCH_READER = 0,
  WATCH_ANALYZER,
  WATCH_PRINTER,
  WATCH_LOGGER,
} cell_in_watchdog_table_T;

extern watchdog_entry_T watchdog_table[4]; /**< register for monitoring thread activity */
void register_in_watchdog(cell_in_watchdog_table_T idx, pthread_t thrd);
void check_in_watcher(cell_in_watchdog_table_T idx);
void* watchdog();

#endif// WATCHER_H
