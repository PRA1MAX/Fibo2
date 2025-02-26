#include <gmp.h>
#include <stdlib.h>
#include "fib_base.h"

static void fast_doubling(mpz_t result, uint64_t n) {
    if (n == 0) {
        mpz_set_ui(result, 0);
        return;
    }

    mpz_t a, b, c, d;
    mpz_inits(a, b, c, d, NULL);
    
    // Initialize base cases
    mpz_set_ui(a, 0);  // F(0) = 0
    mpz_set_ui(b, 1);  // F(1) = 1

    uint64_t mask = 1ULL << (63 - __builtin_clzll(n));  // Highest set bit

    for (; mask; mask >>= 1) {
        // Calculate F(2k) = F(k) * [2 * F(k+1) - F(k)]
        mpz_mul(c, a, a);  // c = F(k)^2
        mpz_mul(d, b, b);  // d = F(k+1)^2
        mpz_add(d, d, c);  // d = F(k)^2 + F(k+1)^2

        if (n & mask) {
            // F(2k+1) = F(k+1)^2 + F(k)^2
            mpz_add(c, c, d);  // c = F(k)^2 + (F(k)^2 + F(k+1)^2)
            mpz_swap(a, d);    // a = F(2k+1)
            mpz_swap(b, c);    // b = F(2k+2)
        } else {
            // F(2k) = F(k) * [2 * F(k+1) - F(k)]
            mpz_sub(d, d, c);  // d = (F(k)^2 + F(k+1)^2) - F(k)^2 = F(k+1)^2
            mpz_swap(a, c);    // a = F(2k)
            mpz_swap(b, d);    // b = F(2k+1)
        }
    }

    mpz_set(result, a);  // Result is in a
    mpz_clears(a, b, c, d, NULL);
}

struct number fibonacci(uint64_t index) {
    mpz_t fib;
    mpz_init(fib);

    if (index == 0) {
        mpz_set_ui(fib, 0);  // F(0) = 0
    } else {
        fast_doubling(fib, index);
    }

    size_t count;
    void *bytes = mpz_export(NULL, &count, -1, 1, 0, 0, fib);

    if (count == 0 && mpz_sgn(fib) == 0) {
        bytes = malloc(1);
        *((unsigned char *)bytes) = 0;
        count = 1;
    }

    struct number ret = {
        .bytes = (unsigned char *)bytes,
        .length = count
    };

    mpz_clear(fib);
    return ret;
}

void fibonacci_cleanup() {
    // No cleanup needed
}
