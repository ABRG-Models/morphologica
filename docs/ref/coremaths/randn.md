---
layout: page
title: morph::Rand*
parent: Core maths classes
grand_parent: Reference
permalink: /ref/coremaths/random/
nav_order: 6
---
```c++
#include <morph/Random.h>
```
Header file: [morph/Random.h](https://github.com/ABRG-Models/morphologica/blob/main/morph/Random.h). Test and example code:  [tests/testRandom](https://github.com/ABRG-Models/morphologica/blob/main/tests/testRandom.cpp) [tests/testRandString](https://github.com/ABRG-Models/morphologica/blob/main/tests/testRandString.cpp)

## Summary

High quality random number generation (RNG) is important in many scientific simulation projects. `<morph/Random.h>` provides a set of random number generation classes. These classes wrap the standard random number generation in the C++ `<random>` header to give a convenient interface for the programmer. While it is possible to select different pseudo random number generators, the classes all default to the very acceptable mt19937 [Mersenne Twister algorithm](https://en.wikipedia.org/wiki/Mersenne_Twister) (using the 32 bit version for `RandUniform`/`RandPoisson` and the 64 bit version for `RandNormal` and `RandLogNormal`).

## Intervals

This page uses the common mathematical notation for intervals:

* **[a,b)** for a *semi-closed interval* which includes values from and including **a** up to, but not including **b**.
* **[a,b]** for a *closed interval* which includes both **a** and **b** in the interval.

## morph::RandUniform

Random number generation from a uniform distribution. Any possible value in the interval is equally likely to be returned. The `RandUniform` class is templated and has integer and floating point specializations:

```c++
namespace morph {
    template <typename T = float, typename E = std::mt19937, bool = std::is_integral<std::decay_t<T>>::value>
    class RandUniform {};
    // Both integer and floating point specializations are implemented
```
`T` is the type for the generated random numbers and `E` is the generator algorithm/engine.

### Simplest usage
```c++
// Floating point RandUniform defaults to the semi-closed interval [0,1)
morph::RandUniform<float> rng1; // Generate random numbers in float type output in interval [0,1)
float f = rng1.get();           // 0 <= f < 1

// By default, an integer RandUniform provides a number from the closed interval [min, max] for the type
morph::RandUniform<unsigned short> rng2;
unsigned short us = rng2.get(); // 0 <= us <= 65535
// Find the end points of the RNG interval with min() and max():
std::cout << "Integer RNG generates numbers in range " << rng2.min() << " to " << rng2.max() << std::endl;
```

### Setting the interval

You can choose an interval over which random numbers should be generated in the constructor
```c++
// Create a generator of integer values in interval [0,10]. The interval type should match
// the RandUniform template type (unsigned int here)
morph::RandUniform<unsigned int> rng (0, 10);

// Create a floating point RNG for the interval [-100, 100)
morph::RandUniform<double> rng (-100.0, 100.0);
```

### Using a fixed seed

To use a fixed seed with the default interval, use a single `unsigned int` argument to the constructor:
```c++
morph::RandUniform<int> rng (2303); // The first .get() should always return the same number now
```

To use a fixed seed with a custom interval, it's the third argument:
```c++
// Declaration
RandUniform (T a, T b, unsigned int _seed);

// In use:
morph::RandUniform<int> rng (0, 100, 2303); // Interval [0,100], seed = 2303
```

### Getting arrays of randomly generated values

If you need to obtain a large number of randomly generated values, it is most efficient to get these into an array, rather than using `RandUniform::get()`` repeatedly.

```c++
morph::RandUniform<double> rng (0.0, 10.0);

// One overload returns std::vector of results
std::vector<double> randoms_in_vector = rng.get (100); // Return a vector with 100 random numbers

// Another places values into an existing std::array
std::array<double, 100> randoms_in_array;
rng.get (randoms_in_array);
```

### Changing the engine

By default the 32 bit mt19937 Mersenne Twister algorithm is used in `RandUniform`. To select a different engine, specify it as a second template parameter.

```c++
morph::RandUniform<float, std::mt19937_64> rng1;  // Use the 64 bit version of mt19937
morph::RandUniform<float, std::minstd_rand> rng2; // Use the linear congruential engine
```
There are some notes about the different engines that you can use in the comments in [morph/Random.h](https://github.com/ABRG-Models/morphologica/blob/main/morph/Random.h). You can also consult [C++ reference material](https://en.cppreference.com/w/cpp/numeric/random).

### One-line RandUniform calls

This is a way to use `RandUniform` in your program with a single line of code. It creates one singleton class that manages the memory used by the `RandUniform<float>` object and one for `RandUniform<double>` and allows you to retrieve random values with `morph::randSingle` and `morph::randDouble` function calls:

```c++
#include <morph/rng.h>
int main() {
    float rnum_single = morph::randSingle();  // single precision or...
    double rnum_double = morph::randDouble(); // double.
    return 0;
}
```
To enable only `randSingle` use `#include <morph/rngs.h>` and to enable only `randDouble`, use `#include <morph/rngd.h>`. There is no equivalent for `RandNormal` or any of the other `Rand*` classes, but it would not be difficult to copy and adapt rng.h if you need this.

## morph::RandNormal and morph::RandLogNormal

Two C++ classes to generate values from either a normal (Gaussian) distribution or a log-normal distribution.

All the examples here show `RandNormal`, but you can substitute `RandLogNormal` if you want the log-normal distribution to generate your values.

```c++
namespace morph {
    template <typename T = double, typename E = std::mt19937_64>
    class RandNormal { // or class RandLogNormal
```
`T` is the type for the generated random numbers and `E` is the generator algorithm/engine.

### Simplest usage

To generate values from a normal distribution with mean 0 and standard deviation 1, create the a default RandNormal object:
```c++
morph::RandNormal<float> rng;
rng.get(); // Getters are the same across the Rand* classes
```

### Specifying the normal distribution parameters

The mean and standard deviation can be specified in the constructor:
```c++
morph::RandNormal<double> rng (1.0, 2.0); // Mean 1, standard deviation 2.
```
### Using a fixed seed

To use a fixed seed with the default interval, use a single `unsigned int` argument to the constructor:
```c++
morph::RandNormal<double> rng (2303); // The first .get() should always return the same number now
```

To use a fixed seed with a custom interval, it's the third argument:
```c++
// Declaration
RandNormal (T a, T b, unsigned int _seed);
// In use:
morph::RandNormal<float> rng (2.0f, 3.0f, 2303); // mean 2, std 3, seed = 2303
```

### Getters

The `get()` function overloads are the same in all the `Rand*` classes, so you can:

```c++
T value = rng.get();
// or
std::vector<T> values = rng.get (num);
// or
std::array<T, N> avals;
rng.get (avals);
```

## morph::RandPoisson

A C++ classes to generate values from a normal Poisson distribution.

```c++
namespace morph {
    template <typename T = int, typename E = std::mt19937>
    class RandPoisson
```
`T` is the integral type for the generated random numbers and `E` is the generator algorithm/engine.

### Simplest usage

To generate values from a Poisson distribution with mean 0, create the a default object:
```c++
morph::RandPoisson<int> rng;
rng.get();
```

### Specifying the distribution mean

The mean can be specified in the constructor:
```c++
morph::RandPoisson<int> rng (3); // Mean 3
```
### Using a fixed seed

To use a fixed seed with the default interval, use a single `unsigned int` argument to the constructor:
```c++
morph::RandPoisson<int> rng (2303); // The first .get() should always return the same number now
```

To use a fixed seed with a custom mean, it's the second argument:
```c++
// Declaration
RandPoisson (T mean, unsigned int _seed);
// In use:
morph::RandPoisson<int> rng (4, 2303); // mean 4, seed = 2303
```

### Getters

The `get()` function overloads are the same as for `RandUniform`, `RandNormal` and `RandLogNormal`.


## morph::RandString

The `RandString` class is a little different from the other `Rand*` classes because it uses a `morph::RandUniform` member to help it generate character strings. It allows you to generate random characters from different character groups such as `morph::CharGroup::AlphaNumeric` or `morph::CharGroup::Decimal`. It is a non-templated class:

```c++
namespace morph {
    class RandString
```

### Simplest usage

To generate 8 HexLowerCase characters:
```c++
morph::RandString string_gen; // Default string length 8, default char group HexLowerCase.
std::string randchars = string_gen.get();
```

You can get any length of string. For the example above, to get another 10 HexLowerCase characters:
```c++
std::string morechars = string_gen.get (10); // updates RandString::length to 10 before getting chars
```

You can also change the character group at runtime:
```c++
string_gen.setCharGroup (morph::CharGroup::AlphaNumeric);
```

### Constructors

```c++
RandString();                  // Default, length set to 8, char group set to HexLowerCase
RandString(const size_t l)     // Construct with length set to l
RandString(const size_t l, const morph::CharGroup& _cg) // Length l, character group _cg
```

The list of possible character groups is given by the `CharGroup` declaration:

```c++
namespace morph {
   enum class CharGroup
    {
        AlphaNumeric,          // 0-9A-Za-z                   62 chars
        Alpha,                 // A-Za-z                      52 chars
        AlphaNumericUpperCase, // 0123456789ABCDEF... ...XYZ  36 chars
        AlphaNumericLowerCase, // 0123456789abcdef... ...xyz  36 chars
        AlphaUpperCase,        // A-Z                         26 chars
        AlphaLowerCase,        // a-z                         26 chars
        HexUpperCase,          // 0123456789ABCDEF            16 chars
        HexLowerCase,          // 0123456789abcdef            16 chars
        Decimal,               // 0123456789                  10 chars
        BinaryTF,              // TF                           2 chars
        Binary                 // 01                           2 chars
    };
}
```
