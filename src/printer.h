#ifndef PRINTER_H
#define PRINTER_H
#include <signal.h>

//for SIGTERM_handler
volatile sig_atomic_t printer_done;
void *printer(void* arg);

#endif // PRINTER_H