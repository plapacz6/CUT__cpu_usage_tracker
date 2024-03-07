#include <signal.h>

volatile sig_atomic_t reader_done = 0;
volatile sig_atomic_t analyzer_done = 0;
volatile sig_atomic_t logger_done = 0;
volatile sig_atomic_t printer_done = 0;
volatile sig_atomic_t watchdog_done = 0;