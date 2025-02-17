#include "morph/Random.h"
#include <iostream>

using namespace std;

int main()
{
    int rtn = 0;

#ifdef __GNUC__
    // GCC not fussy and will accept this even though it defies the standard
    typedef unsigned char SMALL_T;
#else
    // VS enforces the strict minimum size for the std::random code in Random.h, so
    // smallest you can get away with is unsigned short.
    typedef unsigned short SMALL_T;
#endif
    // A random uniform generator returning integer types
    morph::RandUniform<SMALL_T, std::mt19937> rui;
    cout << "Random number is " << static_cast<unsigned int>(rui.get()) << endl;
    // You can find the min and max:
    cout << "That integer unsigned char (or short on Windows) RNG has min and max: " << static_cast<unsigned int>(rui.min())
         << "/" << static_cast<unsigned int>(rui.max()) << endl;

    // A random uniform generator returning real/floating point types
    morph::RandUniform<float, std::mt19937> ruf;
    cout << "Random float number is " << ruf.get() << endl;

    // You can find the min and max:
    cout << "That float RNG has min and max: " << ruf.min() << "/" << ruf.max() << endl;

    // You can get a vector of numbers taken from a random number generator:
    morph::RandUniform<double, std::mt19937_64> rud;
    vector<double> tenrns = rud.get(10);
    cout << "10 random doubles:" << endl;
    for (auto d : tenrns) { cout << d << endl; }

    // You can set up an RNG which has different max and min values: Note use of 64 bit
    // generator even though unsigned int is 32 bits. Will generate a longer sequence of
    // numbers before a repeat.
    morph::RandUniform<unsigned int, std::mt19937_64> rubnd (0, 3);
    // You can find the min and max:
    cout << "That bounded, unsigned integer RNG has min and max: " << rubnd.min() << "/" << rubnd.max() << endl;
    cout << "Ten random unsigned int numbers in that range:\n";
    vector<unsigned int> tenrns2 = rubnd.get(10);
    for (auto d : tenrns2) { cout << d << endl; }

    // There is an overload of get which fills a fixed size array that you pass it with random numbers
    std::array<unsigned int, 12> twelverns;
    rubnd.get(twelverns);
    cout << "Twelve random unsigned int numbers in an array:\n";
    for (auto d : twelverns) { cout << d << endl; }


    // Similar for float
    morph::RandUniform<float, std::mt19937> rubndf (0.0f, 1000.0f, 1);
    // You can find the min and max:
    cout << "FIXED SEED: bounded, float RNG has min and max: " << rubndf.min() << "/" << rubndf.max() << endl;
    cout << "Ten random float numbers in that range:\n";
    vector<float> tenrns3 = rubndf.get(10);
    for (auto d : tenrns3) { cout << d << endl; }

    // Another with seed 1 for float
    morph::RandUniform<float, std::mt19937> rubndf2 (0.0f, 1000.0f, 1);
    cout << "Ten random float numbers in that range from second rng with seed 1:\n";
    vector<float> tenrns32 = rubndf2.get(10);
    for (auto d : tenrns32) { cout << d << endl; }

    // Test two rng generators where no seed is specified
    morph::RandUniform<float, std::mt19937> rubndf3 (0.0f, 1000.0f);
    cout << "Ten random float numbers from the first 'default seed rng':\n";
    vector<float> tenrns33 = rubndf3.get(10);
    for (auto d : tenrns33) { cout << d << endl; }
    morph::RandUniform<float, std::mt19937> rubndf4 (0.0f, 1000.0f);
    cout << "Ten random float numbers from the second 'default seed rng':\n";
    vector<float> tenrns34 = rubndf4.get(10);
    for (auto d : tenrns34) { cout << d << endl; }

    // Normally distributed numbers:
    morph::RandNormal<double, std::mt19937_64> rnorm (5, 0.1);
    vector<double> tennorms = rnorm.get(10);
    cout << "10 random normals (double type):" << endl;
    for (auto d : tennorms) { cout << d << endl; }

    morph::RandLogNormal<double, std::mt19937_64> rln (5, 0.1);
    vector<double> tenlnorms = rln.get(10);
    cout << "10 log normals (double type):" << endl;
    for (auto d : tenlnorms) { cout << d << endl; }

    morph::RandPoisson<int, std::mt19937> rpois (5);
    vector<int> tenpois = rpois.get(10);
    cout << "10 Poisson RNs (int type):" << endl;
    for (auto d : tenpois) { cout << d << endl; }

    return rtn;
}
