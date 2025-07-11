#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <assert.h>
#include "prng/prng.h"
#include "prng/splitmix64.h"


#define NUM_VALUES 100

int main() {
    uint64_t values[NUM_VALUES];

    // First sequence
    prng_seed(42);
    for (int i = 0; i < NUM_VALUES; ++i) {
        values[i] = prng_next();
        printf("first[%03d] = %016" PRIx64 "\n", i, values[i]);
    }

    // Second sequence with same seed
    prng_seed(42);
    for (int i = 0; i < NUM_VALUES; ++i) {
        uint64_t val = prng_next();
        printf("second[%03d] = %016" PRIx64 "\n", i, val);
        assert(val == values[i] && "Mismatch between repeated PRNG sequences");
    }

    printf("All values match. PRNG is reproducible.\n");
    return 0;
}
