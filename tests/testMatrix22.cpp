#include "morph/Matrix22.h"
#include <iostream>
#include <array>
#include <morph/Vector.h>
#include <morph/MathConst.h>

void setMatrixSequence (morph::Matrix22<float>& tm)
{
    tm.mat = { 0, 1, 2, 3 };
}

int main()
{
    int rtn = 0;

    // Test assignment
    morph::Matrix22<float> tm1;
    setMatrixSequence (tm1);
    morph::Matrix22<float> tm2 = tm1;
    std::cout << "After assignment:\n" << tm2 << std::endl;
    for (unsigned int i = 0; i<4; ++i) {
        if (tm2.mat[i] != (float)i) {
            ++rtn;
        }
    }

    // Test determinant
    morph::Matrix22<float> tt;
    tt.mat = { 1.0f, 4.0f, 1.0f, 5.0f };
    float det_td2 = tt.determinant();
    std::cout << "Determinant = " << det_td2 << " (expect 1)" << std::endl;
    if (det_td2 != 1.0f) { ++rtn; }

    // Test matrix inversion
    morph::Matrix22<float> mi;
    mi.mat = { -1, 2, 3, -2 };

    morph::Matrix22<float> miinv = mi.invert();
    std::cout << "mi\n" << mi << std::endl;
    std::cout << "mi.invert():\n" << miinv << std::endl;

    // Test multiplication
    morph::Matrix22<float> mult1;
    setMatrixSequence (mult1);
    std::cout << "mult1\n" << mult1 << std::endl;

    morph::Matrix22<float> mult2;
    mult2.mat = { 5, 4, 3, 2 };
    std::cout << "mult2\n" << mult2 << std::endl;

    morph::Matrix22<float> mult3 = mult1 * mult2;
    std::cout << "mult1 * mult2 =\n" << mult3 << std::endl;

    morph::Matrix22<float> mult3alt = mult1 * mult2.mat;
    std::cout << "mult1 * mult2.mat =\n" << mult3alt << std::endl;

    morph::Matrix22<float> mult2_t = mult2;
    mult2_t.transpose();
    std::cout << "mult2 transposed =\n" << mult2_t << std::endl;

    if (mult3.mat[0] != 8 || mult3.mat[1] != 17
        || mult3.mat[2] != 4|| mult3.mat[3] != 9) {
        ++rtn;
    }

    morph::Matrix22<float> mult1save = mult1;
    mult1 *= mult2;
    std::cout << "mult1 *= mult2 gives\n" << mult1 << std::endl;
    mult1 = mult1save;
    mult1 *= mult2.mat;
    std::cout << "mult1 *= mult2.mat gives\n" << mult1 << std::endl;

    if (mult1.mat[0] != 8 || mult1.mat[1] != 17
        || mult1.mat[2] != 4 || mult1.mat[3] != 9) {
        ++rtn;
    }

    // Vector rotation
    morph::Vector<double, 2> v1 = { 0.0, 0.1 };
    morph::Matrix22<double> rotn;
    rotn.rotate (morph::mathconst<double>::pi_over_3);
    morph::Vector<double, 2> v1_rot = rotn * v1;
    std::cout << "v1: " << v1 << ", rotated pi/3 is: "  << v1_rot << std::endl;

    rotn.rotate (morph::mathconst<double>::two_pi_over_3);
    v1_rot = rotn * v1;
    std::cout << "v1: " << v1 << ", rotated 2pi/3 is: "  << v1_rot << std::endl;

    return rtn;
}
