#ifndef LOGGER_H
#define LOGGER_H
#include "ring_buffer.h"
#include "globals.h"

extern FILE *flog;  /**<  logger file */
extern ring_buffer_t *ptr_logger_buffer;
void* logger(void *ptr_watchdog_place_4_logger);
void log_msg(char *msg);

#endif // LOGGER_H