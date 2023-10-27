/*
 * Example usage of the morph::vvec class.
 *
 * vvec is like std::vector, with maths operations built-in. It makes it convenient
 * to program maths operations on arrays of numbers.
 *
 * Because it is derived from std::vector, you can often use it in place of std::vector.
 */

#include <iostream>
#include <morph/vvec.h>

int main()
{
    // Create and initialize a vvec of floating point numbers:
    morph::vvec<float> vf1 = {1.2f, 3.4f, 7.0f};
    // Create another:
    morph::vvec<float> vf2;
    // Set up the second using the numpy-like linspace function:
    vf2.linspace (0.0f, 1.0f, 3);

    // Output to stdout:
    std::cout << "vf1: " << vf1 << " and vf2: " << vf2 << std::endl;

    // Add them (element wise):
    std::cout << "vf1 + vf2 = " << (vf1 + vf2) << std::endl;

    // Multiply them (element wise - known as the Hadamard product):
    std::cout << "vf1 * vf2 = " << (vf1 * vf2) << std::endl;

    // Add one to a simple scalar number
    std::cout << "vf1 + 4 = " << (vf1 + 4) << std::endl;

    // Raise a vvec to a power:
    std::cout << "vf1 to power 2: " << vf1.pow(2) << std::endl;

    // Find the max of vf1:
    std::cout << "vf1.max() = " << vf1.max() << std::endl;

    // Find the dot product of vf1 and vf2:
    std::cout << "vf1 dot vf2 = " << vf1.dot (vf2) << std::endl;

    // Shuffle vf1.
    std::cout << "Before shuffle vf1 = " << vf1 << std::endl;
    vf1.shuffle();
    std::cout << "After shuffle  vf1 = " << vf1 << std::endl;

    // And vf2
    std::cout << "Before shuffle vf2 = " << vf2 << std::endl;
    std::cout << "After shuffle  vf2 = " << vf2.shuffled() << std::endl;

    std::cout << "\nFor more examples, see morphologica/tests/testvvec.cpp\n";
    return 0;
}
