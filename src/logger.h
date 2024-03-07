#ifndef LOGGER_H
#define LOGGER_H
#include <signal.h>

//for SIGTERM_handler
extern volatile sig_atomic_t logger_done;

void write_log(char who[static 1], char fmt[static 1], ...);
void* logger(void *arg);

#endif // LOGGER_H