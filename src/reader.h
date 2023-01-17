#ifndef READER_H
#define READER_H
#include <stdio.h>
#include <stddef.h>
#include "ring_buffer.h"

//for SIGTERM_handler
extern char* buff_M;
extern FILE *proc_stat_file;
extern char *G_msg_array;
extern ring_buffer_T *rb_ra;
extern char *rb_ra_data_table;

size_t cpu_cors_N(size_t n, int get_);
size_t get_msg_size(size_t n, int get_);
void destroy_buff_M(char* buffM);
int destroy_msg_array(char* msg_a);
int destroy_rb_ra_data_table(char* data_table);
void* reader(void *watchdog_tbl);

#endif // READER_H