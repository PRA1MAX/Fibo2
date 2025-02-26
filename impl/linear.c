#include "fib_base.h"

#ifdef DEBUG
#   define DIGIT uint64_t
#else
#   define DIGIT __uint128_t
#endif

#define DIGIT_BIT (CHAR_BIT * sizeof(DIGIT))

static size_t ndigit_estimate(uint64_t const index)
{
    return (index + DIGIT_BIT - 1) / DIGIT_BIT + 1;
}

static unsigned accumulate(
        DIGIT *restrict a,
        DIGIT const *const b, size_t const ndigits)
{
    unsigned carry = 0;
    for (size_t offset = 0; offset < ndigits; ++offset)
    {
        DIGIT add = b[offset];
        carry = __builtin_add_overflow(add, carry, &add);
        carry += __builtin_add_overflow(a[offset], add, &a[offset]);
    }

    return a[ndigits] = carry;
}

static void swap(DIGIT **lhs, DIGIT **rhs)
{
    DIGIT *tmp = *lhs;
    *lhs = *rhs;
    *rhs = tmp;
}

struct number fibonacci(uint64_t index)
{
    size_t const ndigits_max = ndigit_estimate(index);
    log("Allocating %llu digits of size %llu.\n",
            (long long unsigned)ndigits_max,
            (long long unsigned)sizeof(DIGIT));

    struct number result;
    result.bytes = calloc(2 * ndigits_max, sizeof(DIGIT));
    DIGIT *cur = result.bytes;
    DIGIT *next = &cur[ndigits_max];
    *next = 1;

    size_t ndigits = 1;
    while (index--)
    {
        ndigits += accumulate(next, cur, ndigits);
        swap(&cur, &next);
    }

    result.length = ndigits * sizeof(DIGIT);
    memcpy(result.bytes, cur, result.length);
    return result;
}

