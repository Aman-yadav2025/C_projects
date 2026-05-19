#pragma once

#include "math_utils.hpp"

int multi(int a,int b);
int add(int a, int b);
long long modAdd(long long a,long long b,long long mod = 1e9 + 7);
long long modmulti(long long a,long long b,long long mod = 1e9 + 7);
long long modSubs(long long a,long long b,long long mod = 1e9 + 7);
long long modExpo(long long a,long long b,long long mod = 1e9 + 7);
long long modDiv(long long a,long long b,long long mod = 1e9 + 7);
long long modNCR(long long n, long long r,long long mod = 1e9 + 7)