#include "fib_base.h"
#include <gmp.h>

struct number fibonacci(uint64_t index) {
    
    if (index == 0) {
        struct number zero = { .bytes = malloc(1), .length = 1 };
        ((uint8_t*)zero.bytes)[0] = 0;
        return zero;
    }
    if (index <= 2) {
        struct number one = { .bytes = malloc(1), .length = 1 };
        ((uint8_t*)one.bytes)[0] = 1;
        return one;
    }

    
    mpf_t sqrt5, phi, power, result_float, half;
    mpz_t result;
    struct number ret;
    size_t count;
    mpf_init2(sqrt5, index * 2 + 64);
    mpf_init2(phi, index * 2 + 64);
    mpf_init2(power, index * 2 + 64);
    mpf_init2(result_float, index * 2 + 64);
    mpf_init2(half, 64);
    mpz_init(result);

    mpf_set_ui(sqrt5, 5);
    mpf_sqrt(sqrt5, sqrt5);

    mpf_set_ui(phi, 1);
    mpf_add(phi, phi, sqrt5);
    mpf_div_ui(phi, phi, 2);

    mpf_pow_ui(power, phi, index);

    mpf_div(result_float, power, sqrt5);

    mpf_set_d(half, 0.5);
    mpf_add(result_float, result_float, half);

    mpz_set_f(result, result_float);
    ret.bytes = mpz_export(NULL, &count, -1, 1, 0, 0, result);
    ret.length = count;

    
    mpf_clears(sqrt5, phi, power, result_float, half, NULL);
    mpz_clear(result);

    return ret;
}