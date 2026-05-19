#include <iostream>
#include "math_utils.hpp"

using namespace std;

int main() {
    long long a = 1e9, b = 2e9;
    cout << "Mod Add: " << modAdd(a, b) <<endl;
    cout << "Mod Expo (2^10): " << modExpo(2, 10) <<endl;
    cout <<"10 / 2 mod P = %lld\n"<<modDiv(10, 2, 1000000007)<<endl;

    return 0;
}