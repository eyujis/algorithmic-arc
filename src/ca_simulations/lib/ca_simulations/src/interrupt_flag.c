#include <signal.h>
#include <stdatomic.h>
#include "interrupt_flag.h"

// Use atomic for thread-safe signal handling
static volatile sig_atomic_t stop_requested = 0;

static void handle_sigint(int sig) {
    (void)sig;  // Unused parameter
    stop_requested = 1;
}

void init_interrupt_flag(void) {
    stop_requested = 0;
    signal(SIGINT, handle_sigint);
}

int is_interrupted(void) {
    return stop_requested;
}
