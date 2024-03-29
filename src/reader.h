#ifndef READER_H
#define READER_H
#include <stdio.h>
#include <stddef.h>
#include <signal.h>

//for SIGTERM_handler
extern volatile sig_atomic_t reader_done;

//for main.c
void reader_release_resources(void* arg);

//reader-analyzer
size_t cpu_cors_N(size_t n, int get_);
size_t get_size_msg1core(size_t n, int get_);
char* get_msg_from_rb_ra(void);

//for main()
void* reader(void *arg);
#endif // READER_H