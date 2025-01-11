#include <stdint.h>
#include <stdio.h>

uint64_t factorial_iter(uint32_t n)
{
    uint64_t result = 1;
    for (uint32_t i = 1; i <= n; ++i)
    {
        result *= i;
    }
    return result;
}

uint64_t user_prog(int n) __attribute__((alias("factorialp")));
