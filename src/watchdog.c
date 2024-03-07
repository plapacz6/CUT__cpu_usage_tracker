#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include "logger.h"
#include "watchdog.h"
#include "mutexes.h"

#include <errno.h>

volatile sig_atomic_t watchdog_done = 0;

watchdog_entry_T watchdog_table[WATCH_TBL_SIZE];

void register_in_watchdog(cell_in_watchdog_table_T idx, pthread_t thrd) {
    watchdog_table[idx].ptr_pthread_id = thrd;
    watchdog_table[idx].exists = 1;
    watchdog_table[idx].active = 1;
}

void checkin_watchdog(cell_in_watchdog_table_T idx) {
    watchdog_table[idx].active = 1;
}

void cancel_all_pthreads() {
    int i;
    mtx_lock(&mtx_watchdog);
    for(i = 0; i < WATCH_TBL_SIZE; i++) {
        if(watchdog_table[i].exists) {
            int ret = pthread_cancel(watchdog_table[i].ptr_pthread_id);
            pthread_join(watchdog_table[i].ptr_pthread_id, NULL);
            if(0 != ret ) {
                write_log("cancell_all", "can't cancel phread: %lu\n",
                          watchdog_table[i].ptr_pthread_id);
            }
            watchdog_table[i].exists = 0;
            write_log("cancell_all", "cancellation of phread: %lu",
                      watchdog_table[i].ptr_pthread_id);
        }
        else {
            write_log("cancell_all", "cancel:THREAD DON'D EXISTS: %lu",
                      watchdog_table[i].ptr_pthread_id);
        }
    }
    mtx_unlock(&mtx_watchdog);
}

/**
 * @brief watchdog for 4 threads
 * requires the watchdog_table[4] table to be correctly initialized
 *
 * @return void*
 */
void* watchdog(void * /*arg*/) {
    while(!watchdog_done) {
        sleep(2);
        if(thrd_success == mtx_trylock(&mtx_watchdog)) {
            for(int i = 0; i < 4; i++) {
                if(watchdog_table[i].exists != 0) {
                    if(! watchdog_table[i].active) {

                        int ret = pthread_cancel(watchdog_table[i].ptr_pthread_id);

                        write_log("watchdog", "cancellation of pthread: %lu\n",
                                  watchdog_table[i].ptr_pthread_id);

                        if(0 != ret ) {
                            int errsv = errno;
                            if(errsv == ESRCH)
                                write_log("watchdog", "%s", "cancellation - no such process");
                        }

                        watchdog_table[i].exists = 0;
                    }
                    else {
                        watchdog_table[i].active = 0;
                    }
                }
            }
            mtx_unlock(&mtx_watchdog);
        }//mtx not locked
    }
    write_log("watchdog", "%s", "main loop - done");
    cancel_all_pthreads();
    pthread_exit(0);
}




