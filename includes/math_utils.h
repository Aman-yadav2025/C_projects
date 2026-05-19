#ifndef MATH_UTILS_H
#define MATH_UTILS_H

// Standard versions
int multi(int a, int b);
int add(int a, int b);

// Modular versions (No default values allowed in C)
long long modAdd(long long a, long long b, long long mod);
long long modSubs(long long a, long long b, long long mod);
long long modmulti(long long a, long long b, long long mod);
long long modExpo(long long a, long long b, long long mod);
long long modDiv(long long a,long long b,long long mod);
long long modNCR(long long n, long long r,long long mod)

#endif