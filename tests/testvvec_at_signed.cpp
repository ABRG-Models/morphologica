/*
 * Test the at_signed(I idx) functions
 */

#include <morph/vvec.h>
#include <morph/vec.h>
#include <array>
#include <vector>
#include <set>
#include <cstdint>
#include <iostream>

int main()
{
    int rtn = 0;

    morph::vvec<int> mvf (5, 0);

    mvf.at_signed (-2) = -2;
    mvf.at_signed (-1) = -1;
    mvf.at_signed ( 0) =  0;
    mvf.at_signed ( 1) =  1;
    mvf.at_signed ( 2) =  2;

    std::cout << "Negatively index set vvec: " << mvf << std::endl;

    for (int i = -2; i < 3; ++i) {
        std::cout << "mvf.at_signed("<<i<<") = " << mvf.at_signed(i) << std::endl;
        if (mvf.at_signed(i) != i) { --rtn; }
    }

    for (int i = -2; i < 3; ++i) {
        std::cout << "mvf.c_at_signed("<<i<<") = " << mvf.c_at_signed(i) << std::endl;
        if (mvf.c_at_signed(i) != i) { --rtn; }
    }

    for (int64_t i = -2; i < 3; ++i) {
        std::cout << "mvf.at_signed("<<i<<") = " << mvf.at_signed(i) << std::endl;
        if (mvf.at_signed(i) != i) { --rtn; }
    }

    for (int64_t i = -2; i < 3; ++i) {
        std::cout << "mvf.c_at_signed("<<i<<") = " << mvf.c_at_signed(i) << std::endl;
        if (mvf.c_at_signed(i) != i) { --rtn; }
    }

    for (int16_t i = -2; i < 3; ++i) {
        std::cout << "mvf.at_signed("<<i<<") = " << mvf.at_signed(i) << std::endl;
        if (mvf.at_signed(i) != i) { --rtn; }
    }

    for (int16_t i = -2; i < 3; ++i) {
        std::cout << "mvf.c_at_signed("<<i<<") = " << mvf.c_at_signed(i) << std::endl;
        if (mvf.c_at_signed(i) != i) { --rtn; }
    }

    for (int8_t i = -2; i < 3; ++i) {
        std::cout << "mvf.at_signed(" << static_cast<int>(i) << ") = " << mvf.at_signed(i) << std::endl;
        if (mvf.at_signed(i) != i) { --rtn; }
    }

    for (int8_t i = -2; i < 3; ++i) {
        std::cout << "mvf.c_at_signed(" << static_cast<int>(i) << ") = " << mvf.c_at_signed(i) << std::endl;
        if (mvf.c_at_signed(i) != i) { --rtn; }
    }

    // Should not compile:
#if 0
    for (uint8_t i = 0; i < 6; ++i) {
        std::cout << "mvf.c_at_signed("<<i<<") = " << mvf.c_at_signed(i) << std::endl;
        if (mvf.c_at_signed(i) != i) { --rtn; }
    }
    std::cout << "c_at_signed<unsigned int>: " << mvf.c_at_signed<unsigned int>(1u) << std::endl;
    std::cout << "c_at_signed<float>: " << mvf.c_at_signed<float>(1.0f) << std::endl;
    std::cout << "c_at_signed<double>: " << mvf.c_at_signed<double>(1.0) << std::endl;
#endif

    // Should error
    std::cout << "Expect errors\n";
    try {
        for (int i = 0; i < 7; ++i) {
            std::cout << "mvf.c_at_signed("<<i<<") = " << mvf.c_at_signed(i) << std::endl;
            if (i < 3) {
                if (mvf.c_at_signed(i) != i) { --rtn; }
            } else {
                if (mvf.c_at_signed(i) != i-5) { --rtn; }
            }
        }
        --rtn;
    } catch (const std::out_of_range& e) {
        std::cout << "Caught expected out of range: " << e.what() << std::endl;
    }

    try {
        if (mvf.c_at_signed(-6) != -6) { --rtn; }
        --rtn;
    } catch (const std::out_of_range& e) {
        std::cout << "Caught expected out of range: " << e.what() << std::endl;
    }

    std::cout << "Test is returning: " << rtn << std::endl;
    return rtn;
}
