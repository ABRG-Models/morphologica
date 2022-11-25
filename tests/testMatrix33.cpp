#include "morph/Matrix33.h"
#include <iostream>
#include <array>
#include <morph/Vector.h>

void setMatrixSequence (morph::Matrix33<float>& tm)
{
    tm.mat[0] = 0;
    tm.mat[1] = 1;
    tm.mat[2] = 2;
    tm.mat[3] = 3;
    tm.mat[4] = 4;
    tm.mat[5] = 5;
    tm.mat[6] = 6;
    tm.mat[7] = 7;
    tm.mat[8] = 8;
}

int main()
{
    int rtn = 0;

    // Test assignment
    morph::Matrix33<float> tm1;
    setMatrixSequence (tm1);
    morph::Matrix33<float> tm2 = tm1;
    std::cout << "After assignment:\n" << tm2 << std::endl;
    for (unsigned int i = 0; i<9; ++i) {
        if (tm2.mat[i] != (float)i) {
            ++rtn;
        }
    }

    // Test 2x2 determinant
    morph::Matrix33<float> td;
    std::array<float, 4> twotwo = { 1.0f, 4.0f, 1.0f, 5.0f };
    float det_td = td.determinant (twotwo);
    std::cout << "Determinant = " << det_td << " (expect 1)" << std::endl;
    if (det_td != 1.0f) { ++rtn; }

    // Test 3x3 determinant
    std::array<float, 9> threethree = { 1.0f, 0.0f, 2.0f, 1.0f, 1.0f, 3.5f, 3.0f, 2.0f, 120.0f };
    float det_td2 = td.determinant (threethree);
    std::cout << "Determinant = " << det_td2 << " (expect 111)" << std::endl;
    if (det_td2 != 111.0f) { ++rtn; }

    // Test matrix inversion
    morph::Matrix33<float> mi;
    mi.mat[0] = -1;
    mi.mat[1] = 2 ;
    mi.mat[2] = 3;
    mi.mat[3] = -2;
    mi.mat[4] = 1;
    mi.mat[5] = 4;
    mi.mat[6] = 2;
    mi.mat[7] = 1;
    mi.mat[8] = 5; // This is Sal's example from Kahn academy!

    morph::Matrix33<float> miinv = mi.invert();
    std::cout << "mi\n" << mi << std::endl;
    std::cout << "mi.invert():\n" << miinv << std::endl;

    // Test multiplication
    morph::Matrix33<float> mult1;
    setMatrixSequence (mult1);
    std::cout << "mult1\n" << mult1 << std::endl;

    morph::Matrix33<float> mult2;
    mult2.mat[0] = 15;
    mult2.mat[1] = 14;
    mult2.mat[2] = 13;
    mult2.mat[3] = 12;
    mult2.mat[4] = 11;
    mult2.mat[5] = 10;
    mult2.mat[6] = 9;
    mult2.mat[7] = 8;
    mult2.mat[8] = 7;
    std::cout << "mult2\n" << mult2 << std::endl;

    morph::Matrix33<float> mult3 = mult1 * mult2;
    std::cout << "mult1 * mult2 =\n" << mult3 << std::endl;

    morph::Matrix33<float> mult3alt = mult1 * mult2.mat;
    std::cout << "mult1 * mult2.mat =\n" << mult3alt << std::endl;

    morph::Matrix33<float> mult2_t = mult2;
    mult2_t.transpose();
    std::cout << "mult2 transposed =\n" << mult2_t << std::endl;

    if (mult3.mat[0] != 120
        || mult3.mat[1] != 162
        || mult3.mat[2] != 204
        || mult3.mat[3] != 93
        || mult3.mat[4] != 126
        || mult3.mat[5] != 159
        || mult3.mat[6] != 66
        || mult3.mat[7] != 90
        || mult3.mat[8] != 114
        ) {
        ++rtn;
    }
    morph::Matrix33<float> mult1save = mult1;
    mult1 *= mult2;
    std::cout << "mult1 *= mult2 gives\n" << mult1 << std::endl;
    mult1 = mult1save;
    mult1 *= mult2.mat;
    std::cout << "mult1 *= mult2.mat gives\n" << mult1 << std::endl;

    if (mult1.mat[0] != 120
        || mult1.mat[1] != 162
        || mult1.mat[2] != 204
        || mult1.mat[3] != 93
        || mult1.mat[4] != 126
        || mult1.mat[5] != 159
        || mult1.mat[6] != 66
        || mult1.mat[7] != 90
        || mult1.mat[8] != 114
        ) {
        ++rtn;
    }

    return rtn;
}
