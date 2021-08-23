/*
 * Get random numbers from the uniform random number generator convenience functions
 * morph::randSingle() and morph::randDoubple().
 */

#include <morph/rng.h>
#include <iostream>
int main()
{
    std::cout << "Random floating point number: " << morph::randSingle() << std::endl;
    std::cout << "Random double precision number: " << morph::randDouble() << std::endl;

    std::cout << "Second random floating point number: " << morph::randSingle() << std::endl;
    std::cout << "Second random double precision number: " << morph::randDouble() << std::endl;

    return 0;
}
