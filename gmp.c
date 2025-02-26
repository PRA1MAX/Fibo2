#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include <pthread.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

typedef struct {
    struct timespec start;
} MyTimer;

void my_timer_start(MyTimer *timer) {
    clock_gettime(CLOCK_MONOTONIC, &timer->start);
}

double my_timer_get_ms(MyTimer *timer) {
    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);
    return (end.tv_sec - timer->start.tv_sec) * 1000.0 +
           (end.tv_nsec - timer->start.tv_nsec) / 1000000.0;
}

// GMP Fibonacci
void gmp_fibo(mpz_t result, int n) {
    mpz_fib_ui(result, n);
}

// Dynamic programming Fibonacci
typedef struct {
    int key;
    mpz_t value;
} DpEntry;

DpEntry *dp = NULL;
int dp_size = 0;
int dp_capacity = 0;

void dp_init() {
    dp_capacity = 10;
    dp = (DpEntry *)malloc(dp_capacity * sizeof(DpEntry));
    dp_size = 0;
}

void dp_add(int key, const mpz_t value) {
    if (dp_size >= dp_capacity) {
        dp_capacity *= 2;
        dp = (DpEntry *)realloc(dp, dp_capacity * sizeof(DpEntry));
    }
    dp[dp_size].key = key;
    mpz_init(dp[dp_size].value);
    mpz_set(dp[dp_size].value, value);
    dp_size++;
}

mpz_t *dp_get(int key) {
    for (int i = 0; i < dp_size; i++) {
        if (dp[i].key == key) {
            return &dp[i].value;
        }
    }
    return NULL;
}

void F(mpz_t result, int n) {
    if (n <= 2) {
        mpz_set_ui(result, 1);
        return;
    }
    mpz_t *cached = dp_get(n);
    if (cached) {
        mpz_set(result, *cached);
        return;
    }

    int k = n / 2;
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
        mpz_add_ui(result, result, 2 * sign);
        mpz_clear(term1);
        mpz_clear(term2);
    }
    dp_add(n, result);
    mpz_clear(Fk);
    mpz_clear(Fk1);
}

void dp_fibo(mpz_t result, int n) {
    F(result, n);
}

// Best Fibonacci with optimized calculation
typedef struct {
    int *keys;
    int size;
    int capacity;
} KeyList;

void keylist_init(KeyList *list) {
    list->capacity = 10;
    list->keys = (int *)malloc(list->capacity * sizeof(int));
    list->size = 0;
}

void keylist_add(KeyList *list, int key) {
    for (int i = 0; i < list->size; i++) {
        if (list->keys[i] == key) return;
    }
    if (list->size >= list->capacity) {
        list->capacity *= 2;
        list->keys = (int *)realloc(list->keys, list->capacity * sizeof(int));
    }
    list->keys[list->size++] = key;
}

void list_dependency(KeyList *list, int n) {
    if (n <= 1) return;
    keylist_add(list, n);
    list_dependency(list, n / 2);
    list_dependency(list, (n / 2) - 1);
}

typedef struct {
    mpz_t *num;
} SquareArgs;

void *square_thread(void *args) {
    SquareArgs *sa = (SquareArgs *)args;
    mpz_mul(*(sa->num), *(sa->num), *(sa->num));
    return NULL;
}

void best_fibo(mpz_t result, int n) {
    if (n <= 100) {
        gmp_fibo(result, n);
        return;
    }

    KeyList dependencies;
    keylist_init(&dependencies);
    list_dependency(&dependencies, n);

    mpz_t f0, f1, f2, dummy;
    mpz_inits(f0, f1, f2, dummy, NULL);

    MyTimer timer;
    my_timer_start(&timer);

    bool started = false;
    for (int i = 0; i < dependencies.size; i++) {
        int N = dependencies.keys[i];
        if (N < 20) continue;

        if (!started) {
            gmp_fibo(f0, N - 1);
            gmp_fibo(f1, N);
            mpz_add(f2, f0, f1);
            started = true;
            continue;
        }

        int k = N / 2;
        int sign = (k % 2 == 0) ? 1 : -1;

        if (N % 2 == 1) {
            SquareArgs args = { &f1 };
            pthread_t thread;
            pthread_create(&thread, NULL, square_thread, &args);
            mpz_mul(f0, f0, f0);
            pthread_join(thread, NULL);

            mpz_mul_2exp(dummy, f1, 2);
            mpz_sub(dummy, dummy, f0);
            mpz_add_ui(dummy, dummy, 2 * sign);
            mpz_add(f0, f1, f0);
            mpz_sub(f1, dummy, f0);
        } else {
            SquareArgs args = { &f1 };
            pthread_t thread;
            pthread_create(&thread, NULL, square_thread, &args);
            mpz_mul(f0, f0, f0);
            pthread_join(thread, NULL);

            mpz_mul_2exp(dummy, f1, 2);
            mpz_sub(dummy, dummy, f0);
            mpz_add_ui(dummy, dummy, 2 * sign);
            mpz_add(f0, f1, f0);
            mpz_sub(f1, dummy, f0);
        }
    }

    int k = n / 2;
    int sign = (k % 2 == 0) ? 1 : -1;
    if (n % 2 == 0) {
        mpz_mul_2exp(dummy, f0, 1);
        mpz_add(dummy, f1, dummy);
        mpz_mul(result, f1, dummy);
    } else {
        mpz_mul_2exp(dummy, f1, 1);
        mpz_add(dummy, dummy, f0);
        mpz_mul(result, dummy, f1);
        mpz_sub_ui(result, result, 2 * sign);
    }

    mpz_clears(f0, f1, f2, dummy, NULL);
    free(dependencies.keys);
}

// Binet's formula
void binet_fibo(mpz_t result, int n) {
    mpf_t sqrt5, phi, power, result_float;
    mpf_init2(sqrt5, n + 64);
    mpf_init2(phi, n + 64);
    mpf_init2(power, n + 64);
    mpf_init2(result_float, n + 64);

    mpf_set_ui(sqrt5, 5);
    mpf_sqrt(sqrt5, sqrt5);
    mpf_add_ui(phi, sqrt5, 1);
    mpf_div_ui(phi, phi, 2);

    mpf_pow_ui(power, phi, n);
    mpf_div(result_float, power, sqrt5);
    mpf_add_ui(result_float, result_float, 0.5);

    mpz_set_f(result, result_float);

    mpf_clears(sqrt5, phi, power, result_float, NULL);
}

// Matrix Fibonacci
typedef struct {
    mpz_t data[2][2];
} Matrix;

void matrix_mult(Matrix *res, const Matrix *a, const Matrix *b) {
    mpz_t temp[2][2];
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            mpz_init(temp[i][j]);
            mpz_mul(temp[i][j], a->data[i][0], b->data[0][j]);
            mpz_addmul(temp[i][j], a->data[i][1], b->data[1][j]);
        }
    }
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            mpz_swap(res->data[i][j], temp[i][j]);
            mpz_clear(temp[i][j]);
        }
    }
}

void matrix_pow(Matrix *res, Matrix *base, int exponent) {
    Matrix temp_mat;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            mpz_init_set_ui(res->data[i][j], (i == j) ? 1 : 0);
        }
    }

    Matrix temp;
    while (exponent > 0) {
        if (exponent % 2 == 1) {
            matrix_mult(&temp_mat, res, base);
            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < 2; j++) {
                    mpz_swap(res->data[i][j], temp_mat.data[i][j]);
                }
            }
        }
        matrix_mult(&temp, base, base);
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 2; j++) {
                mpz_swap(base->data[i][j], temp.data[i][j]);
            }
        }
        exponent /= 2;
    }
}

void matrix_fibo(mpz_t result, int n) {
    if (n <= 2) {
        mpz_set_ui(result, 1);
        return;
    }
    n--;
    Matrix pow, res;
    mpz_init_set_ui(pow.data[0][0], 1);
    mpz_init_set_ui(pow.data[0][1], 1);
    mpz_init_set_ui(pow.data[1][0], 1);
    mpz_init_set_ui(pow.data[1][1], 0);

    mpz_init_set_ui(res.data[0][0], 1);
    mpz_init_set_ui(res.data[0][1], 0);
    mpz_init_set_ui(res.data[1][0], 0);
    mpz_init_set_ui(res.data[1][1], 1);

    matrix_pow(&res, &pow, n);
    mpz_set(result, res.data[0][0]);

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            mpz_clear(pow.data[i][j]);
            mpz_clear(res.data[i][j]);
        }
    }
}

// Testing functions
bool test_single(int n) {
    MyTimer timer;
    mpz_t res1, res2, res3, res4, res5;
    mpz_inits(res1, res2, res3, res4, res5, NULL);

    my_timer_start(&timer);
    dp_fibo(res2, n);
    double cost2 = my_timer_get_ms(&timer);
    printf("dp cost = %.2f ms\n", cost2);

    my_timer_start(&timer);
    gmp_fibo(res1, n);
    double cost1 = my_timer_get_ms(&timer);
    printf("gmp cost = %.2f ms\n", cost1);

    my_timer_start(&timer);
    best_fibo(res3, n);
    double cost3 = my_timer_get_ms(&timer);
    printf("best cost = %.2f ms\n", cost3);

    my_timer_start(&timer);
    binet_fibo(res4, n);
    double cost4 = my_timer_get_ms(&timer);
    printf("binet cost = %.2f ms\n", cost4);

    my_timer_start(&timer);
    matrix_fibo(res5, n);
    double cost5 = my_timer_get_ms(&timer);
    printf("matrix cost = %.2f ms\n", cost5);

    bool ok = true;
    if (mpz_cmp(res1, res2) != 0) {
        printf("DP wrong answer\n");
        ok = false;
    }
    if (mpz_cmp(res1, res3) != 0) {
        printf("Best wrong answer\n");
        ok = false;
    }
    if (mpz_cmp(res1, res4) != 0) {
        printf("Binet wrong answer\n");
        ok = false;
    }
    if (mpz_cmp(res1, res5) != 0) {
        printf("Matrix wrong answer\n");
        ok = false;
    }

    char *s1 = mpz_get_str(NULL, 10, res1);
    FILE *fo = fopen("output.txt", "w");
    fprintf(fo, "%s\n", s1);
    fclose(fo);
    free(s1);

    mpz_clears(res1, res2, res3, res4, res5, NULL);
    return ok;
}

int main(int argc, char *argv[]) {
    int L = (argc > 1) ? atoi(argv[1]) : 10000000;
    int R = (argc > 2) ? atoi(argv[2]) : L;
    R = (R > L) ? R : L;

    dp_init();
    mpz_t tmp;
    mpz_init_set_ui(tmp, 0);
    dp_add(0, tmp);
    mpz_set_ui(tmp, 1);
    dp_add(1, tmp);
    dp_add(2, tmp);
    mpz_set_ui(tmp, 2);
    dp_add(3, tmp);
    mpz_clear(tmp);

    bool result;
    if (L == R) {
        result = test_single(L);
    } else {
        // Implement range test if needed
        result = false;
    }

    for (int i = 0; i < dp_size; i++) {
        mpz_clear(dp[i].value);
    }
    free(dp);

    printf(result ? "Correct\n" : "Wrong\n");
    return 0;
}