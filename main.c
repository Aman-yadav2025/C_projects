#include <stdio.h>
#include "math_utils.h"

int main() {
    // 1e9 and 2e9 are doubles, so we cast or use literal form
    long long a = 1000000000; 
    long long b = 2000000000;
    long long mod = 1000000007;

    // In C, we must explicitly pass the 'mod' argument
    printf("Mod Add: %lld\n", modAdd(a, b, mod));
    printf("Mod Expo (2^10): %lld\n", modExpo(2, 10, mod));
    printf("10 / 2 mod P = %lld\n", modDiv(10, 2, 1000000007));
    
    return 0;
}