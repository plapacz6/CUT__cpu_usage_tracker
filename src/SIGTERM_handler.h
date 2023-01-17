#ifndef SIGTERM_HANDLER_H
#define SIGTERM_HANDLER_H
#include <stdio.h>

void SIGTERM_handler(int signum);
void install_SIGTERM_handler();
#endif // SIGTERM_HANDLER_H