#ifndef XOSHIRO256PP_H
#define XOSHIRO256PP_H
#include <stdint.h>

uint64_t xoshiro256plusplus_next(void);
void xoshiro256plusplus_set_state(int index, uint64_t value);

#endif
