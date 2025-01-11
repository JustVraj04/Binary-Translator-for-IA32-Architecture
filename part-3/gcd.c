#include <stdint.h>
#include <stdio.h>

uint64_t gcd(uint64_t a, uint64_t b)
{
    while (b != 0)
    {
        uint64_t t = b;
        b = a % b;
        a = t;
    }
    return a;
}

uint64_t user_prog(uint64_t *args) __attribute__((alias("gcd")));
