#ifndef MUTEXES_H
#define MUTEXES_H

#include <threads.h>

extern mtx_t mtx_watchdog;
extern mtx_t mtx_logger;
extern mtx_t mtx_reader_analyzer;
extern mtx_t mtx_analyzer_printer;  
extern cnd_t cnd_ra;
extern cnd_t cnd_ap;
extern cnd_t cnd_log;

#endif //MUTEXES_H