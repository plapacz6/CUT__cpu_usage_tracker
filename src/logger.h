#ifndef LOGGER_H
#define LOGGER_H

void close_log_file();
void destroy_logger_buffer();
void write_log(char who[static 1], char fmt[static 1], ...);
void* logger(void *arg);

#endif // LOGGER_H