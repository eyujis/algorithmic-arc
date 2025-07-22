#include "prng/xoshiro256plusplus.h"
#include "prng/splitmix64.h"


static uint64_t seed_state;

void prng_seed(uint64_t seed) {
    seed_state = seed;
    for (int i = 0; i < 4; ++i)
        xoshiro256plusplus_set_state(i, splitmix64_next(&seed_state));
}

uint64_t prng_next() {
    return xoshiro256plusplus_next();
}

uint8_t prng_next_byte() {
    return prng_next() & 0xFF;
}
