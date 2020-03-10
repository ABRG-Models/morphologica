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
    for (auto d : tenrns) { cout << d << endl; }

    // You can set up an RNG which has different max and min values:
    morph::RandUniformInt<unsigned int> rubnd (0, 3);
    // You can find the min and max:
    cout << "That bounded, integer RNG has min and max: " << rubnd.min() << "/" << rubnd.max() << endl;
    cout << "Ten random numbers in that range:\n";
    vector<unsigned int> tenrns2 = rubnd.get(10);
    for (auto d : tenrns2) { cout << d << endl; }

    // Similar for float
    morph::RandUniformReal<float> rubndf (0.0f, 1000.0f);
    // You can find the min and max:
    cout << "That bounded, float RNG has min and max: " << rubndf.min() << "/" << rubndf.max() << endl;
    cout << "Ten random numbers in that range:\n";
    vector<float> tenrns3 = rubndf.get(10);
    for (auto d : tenrns3) { cout << d << endl; }

    // Normally distributed numbers:
    morph::RandNormal<double> rnorm (5, 0.1);
    vector<double> tennorms = rnorm.get(10);
    cout << "10 normals:" << endl;
    for (auto d : tennorms) { cout << d << endl; }

    return rtn;
}
