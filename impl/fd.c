#include <gmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#ifndef FD_H
#define FD_H



void fibonacci(mpz_t result, const mpz_t n);

#endif
typedef struct {
    int key;
    mpz_t value;
} TempEntry;

static void mpz_rsblsh2(mpz_t res, mpz_t a, mpz_t b) {
    mpz_t tmp;
    mpz_init(tmp);
    mpz_mul_2exp(tmp, a, 2);  // tmp = 4*a
    mpz_sub(res, tmp, b);      // res = 4*a - b
    mpz_clear(tmp);
}

static void fast_doubling_core(int n, TempEntry* temps, int* temp_size);

void mpz_fibo(mpz_t result, int n) {
    if (n <= 100) {
        mpz_fib_ui(result, n);
        return;
    }

    TempEntry* temps = malloc((n + 2) * sizeof(TempEntry));
    int temp_size = 0;
    
    // Initialize base cases
    mpz_init_set_ui(temps[0].value, 0); temps[0].key = 0; temp_size++;
    mpz_init_set_ui(temps[1].value, 1); temps[1].key = 1; temp_size++;
    mpz_init_set_ui(temps[2].value, 1); temps[2].key = 2; temp_size++;

    fast_doubling_core(n, temps, &temp_size);

    // Find and return the result
    for (int i = 0; i < temp_size; i++) {
        if (temps[i].key == n) {
            mpz_set(result, temps[i].value);
            break;
        }
    }

    // Cleanup
    for (int i = 0; i < temp_size; i++) {
        mpz_clear(temps[i].value);
    }
    free(temps);
}

static void fast_doubling_core(int n, TempEntry* temps, int* temp_size) {
    if (n <= 2) return;

    int k = n / 2;
    fast_doubling_core(k, temps, temp_size);
    fast_doubling_core(k - 1, temps, temp_size);

    // Find F(k) and F(k-1) in temps
    mpz_t Fk, Fk1;
    mpz_init(Fk); 
    mpz_init(Fk1);
    
    for (int i = 0; i < *temp_size; i++) {
        if (temps[i].key == k) mpz_set(Fk, temps[i].value);
        if (temps[i].key == k-1) mpz_set(Fk1, temps[i].value);
    }

    mpz_t a, b, c;
    mpz_inits(a, b, c, NULL);
    int sign = (k % 2 == 0) ? 1 : -1;

    if (n % 2 == 1) {
        // Calculate F[2k+1]
        mpz_pow_ui(a, Fk, 2);        // Fk^2
        mpz_mul_2exp(a, a, 2);       // 4*Fk^2
        mpz_pow_ui(b, Fk1, 2);       // Fk1^2
        mpz_sub(a, a, b);            // 4Fk² - Fk1²
        mpz_add_ui(a, a, (sign > 0) ? 2 : -2);  // +2*(-1)^k

        // Calculate F[2k-1]
        mpz_pow_ui(b, Fk, 2);
        mpz_pow_ui(c, Fk1, 2);
        mpz_add(b, b, c);            // Fk² + Fk1²

        // Store results
        temps[*temp_size].key = 2*k + 1;
        mpz_init_set(temps[*temp_size].value, a);
        (*temp_size)++;
        
        temps[*temp_size].key = 2*k - 1;
        mpz_init_set(temps[*temp_size].value, b);
        (*temp_size)++;
    } else {
        // Calculate F[2k]
        mpz_t tmp;
        mpz_init(tmp);
        mpz_mul_2exp(tmp, Fk1, 1);  // 2*Fk1
        mpz_add(tmp, Fk, tmp);       // Fk + 2Fk1
        mpz_mul(a, Fk, tmp);         // Fk*(Fk + 2Fk1)

        // Store result
        temps[*temp_size].key = 2*k;
        mpz_init_set(temps[*temp_size].value, a);
        (*temp_size)++;
        
        mpz_clear(tmp);
    }

    mpz_clears(Fk, Fk1, a, b, c, NULL);
}

// Example usage
int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s <n>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    mpz_t result;
    mpz_init(result);

    mpz_fibo(result, n);
    
    printf("F(%d) = ", n);
    mpz_out_str(stdout, 10, result);
    printf("\n");

    mpz_clear(result);
    return 0;
}
