#include <gmp.h>
#include <stdlib.h>
#include "fib_base.h"

typedef struct {
    uint64_t key;
    mpz_t value;
} DpEntry;

DpEntry *dp = NULL;
int dp_size = 0;
int dp_capacity = 0;

static void dp_init() {
    dp_capacity = 10;
    dp = (DpEntry *)malloc(dp_capacity * sizeof(DpEntry));
    dp_size = 0;
}

static void dp_add(uint64_t key, const mpz_t value) {
    if (dp_size >= dp_capacity) {
        dp_capacity *= 2;
        dp = (DpEntry *)realloc(dp, dp_capacity * sizeof(DpEntry));
    }
    dp[dp_size].key = key;
    mpz_init(dp[dp_size].value);
    mpz_set(dp[dp_size].value, value);
    dp_size++;
}

static mpz_t *dp_get(uint64_t key) {
    for (int i = 0; i < dp_size; i++) {
        if (dp[i].key == key) {
            return &dp[i].value;
        }
    }
    return NULL;
}

static void F(mpz_t result, uint64_t n) {
    if (n == 0) {
        mpz_set_ui(result, 0);
        return;
    } else if (n <= 2) {
        mpz_set_ui(result, 1);
        return;
    }
    mpz_t *cached = dp_get(n);
    if (cached) {
        mpz_set(result, *cached);
        return;
    }

    uint64_t k = n / 2;
    mpz_t Fk, Fk1;
    mpz_init(Fk);
    mpz_init(Fk1);
    F(Fk, k);
    F(Fk1, k - 1);

    if (n % 2 == 0) {
        mpz_t temp;
        mpz_init(temp);
        mpz_mul_ui(temp, Fk1, 2);
        mpz_add(temp, Fk, temp);
        mpz_mul(result, Fk, temp);
        mpz_clear(temp);
    } else {
        mpz_t term1, term2;
        mpz_init(term1);
        mpz_init(term2);
        mpz_mul_ui(term1, Fk, 2);
        mpz_add(term1, term1, Fk1);
        mpz_mul_ui(term2, Fk, 2);
        mpz_sub(term2, term2, Fk1);
        mpz_mul(result, term1, term2);
         int sign = (k % 2 == 0) ? 1 : -1;
        if (sign == 1) {
            mpz_add_ui(result, result, 2);
        } else {
            mpz_sub_ui(result, result, 2);
        }

        mpz_clear(term1);
        mpz_clear(term2);
    }
    dp_add(n, result);
    mpz_clear(Fk);
    mpz_clear(Fk1);
}

struct number fibonacci(uint64_t index) {
    static int dp_initialized = 0;
    if (!dp_initialized) {
        dp_init();
        dp_initialized = 1;
    }

    mpz_t fib;
    mpz_init(fib);
    F(fib, index);

    size_t count;
    void *bytes = mpz_export(NULL, &count, -1, 1, 0, 0, fib);

    // Handle zero (mpz_export returns NULL for zero)
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