#include "Random.h"
#include <iostream>

using namespace std;

int main()
{
    int rtn = 0;

    // A random uniform generator returning integer types
    morph::RandUniformInt<unsigned char> rui;
    cout << "Random number is " << (unsigned int)rui.get() << endl;
    // You can find the min and max:
    cout << "That integer RNG has min and max: " << (unsigned int)rui.min()
         << "/" << (unsigned int)rui.max() << endl;

    // A random uniform generator returning real/floating point types
    morph::RandUniformReal<float> ruf;
    cout << "Random float number is " << ruf.get() << endl;

    // You can find the min and max:
    cout << "That float RNG has min and max: " << ruf.min() << "/" << ruf.max() << endl;

    // You can get a vector of numbers taken from a random number generator:
    morph::RandUniformReal<double> rud;
    vector<double> tenrns = rud.get(10);
    cout << "10 random doubles:" << endl;
    for (auto d : tenrns) {
        cout << d << endl;
    }

    return rtn;
}
