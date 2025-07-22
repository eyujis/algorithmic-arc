#ifndef PRNG_H
#define PRNG_H

#include <stdint.h>
#include "prng/xoshiro256plusplus.h"
#include "prng/splitmix64.h"

void prng_seed(uint64_t seed);
uint64_t prng_next();
uint8_t prng_next_byte();

#endif
