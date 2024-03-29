#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <threads.h>
#include <pthread.h>
#include <assert.h>
#include <signal.h>

#include "logger.h"
#include "printer.h"
#include "reader.h"  //cpu_N
#include "analyzer.h"
#include "watchdog.h"
#include "mutexes.h"


int printer_debug_on = 0;

volatile sig_atomic_t printer_done = 0;

enum {BAR_SIZE = 81};
void bar_create(char* bar, long double val);

void *printer(void* /*arg*/) {

    // size_t msg_size = get_size_msg1core(0, 0);
    size_t cpu_CorN = cpu_cors_N(0, 0);

    char bar[BAR_SIZE] = {};

    while(!printer_done) {
        mtx_lock(&mtx_analyzer_printer);
        cnd_wait(&cnd_ap, &mtx_analyzer_printer);
        assert(ptr_avr != NULL);

        if(!printer_debug_on) system("clear");
        printf("%s\n","printer:");  //DEBUG  ""
        printf("%s\n","average usage cpu:");

        for(size_t i = 0; i < cpu_CorN; ++i) {
            bar_create(bar, ptr_avr[i]);
            if(i == 0) {
                printf("\tcpu: %12.2Lf%%  %s\n", ptr_avr[i], bar);
            }
            else {
                printf("\tcpu%02zu: %10.2Lf%%  %s\n", i, ptr_avr[i], bar);
            }
            fflush(stdout);
        }//for cpu_CorN

        mtx_unlock(&mtx_analyzer_printer);

        //menu ctl+c = end
        printf("%s", "\n\n\n");
        printf("%s\n", "ctl + c = exit");

        mtx_lock(&mtx_watchdog);
        checkin_watchdog(WATCH_PRINTER);
        mtx_unlock(&mtx_watchdog);

        //sleep(1);
    }//while(1)
    write_log("printer","%s","pthread finished");
    pthread_exit(0);
}

void bar_create(char* bar, long double val) {
    int full_len = 54-8; //46
    int val_len = (full_len * val)/100;
    assert(val_len < BAR_SIZE);
    int i;
    //memset(bar, 0, BAR_SIZE);
    for(i = 0; i < val_len; i++) {
        bar[i] = 0x7c; //'!'; //'='; //'|'; //'*'; //'I'; //'X';
    }
    bar[i] = '\0';
}