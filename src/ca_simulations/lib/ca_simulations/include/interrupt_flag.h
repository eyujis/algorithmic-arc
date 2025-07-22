#ifndef INTERRUPT_FLAG_H
#define INTERRUPT_FLAG_H

// Initializes SIGINT handler
void init_interrupt_flag(void);

// Checks if SIGINT was received
int is_interrupted(void);

#endif
