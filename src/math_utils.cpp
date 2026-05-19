#include "math_utils.hpp"

int multi(int a,int b){
    return a*b;
}

int add(int a,int b){
    return a+b;
}

long long fact[1000005];

void precomputeFactorials(int n, long long mod) {
    fact[0] = 1;
    for (int i = 1; i <= n; i++) {
        fact[i] = modmulti(fact[i - 1], i, mod);
    }
}

long long modAdd(long long a,long long b,long long mod)
{
    return(((a%mod)+(b%mod))%mod);
}

long long modSubs(long long a, long long b, long long mod) {
    return ((a % mod) - (b % mod) + mod) % mod;
}

// Multiplication: (a * b) % mod
long long modmulti(long long a, long long b, long long mod) {
    return ((a % mod) * (b % mod)) % mod;
}

long long modExpo(long long a, long long b, long long mod){
    long long ans = 1ll;
    while(b){
        a%= mod;
        if(b%2){
            ans = ans*a %mod;
            b-=1;
            continue;
        }
        a = a*a %mod;
        b/=2;
    }
    return ans;
}

long long modDiv(long long a,long long b,long long mod){
    long long temp;
    temp = modExpo(b,mod-2,mod);
    return (((a%mod)*(temp%mod))%mod);
}

long long modNCR(long long n, long long r, long long mod) {
    if (r < 0 || r > n) return 0;
    long long num = fact[n];
    long long den = modmulti(fact[r], fact[n - r], mod);
    return modDiv(num, den, mod);
}