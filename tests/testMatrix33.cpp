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

#if 0
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
    mult2.mat[9] = 6;
    mult2.mat[10] = 5;
    mult2.mat[11] = 4;
    mult2.mat[12] = 3;
    mult2.mat[13] = 2;
    mult2.mat[14] = 1;
    mult2.mat[15] = 0;
    std::cout << "mult2\n" << mult2 << std::endl;

    morph::Matrix33<float> mult3 = mult1 * mult2;
    std::cout << "mult1 * mult2 =\n" << mult3 << std::endl;

    if (mult3.mat[0] != 304
        || mult3.mat[1] != 358
        || mult3.mat[2] != 412
        || mult3.mat[3] != 466
        || mult3.mat[4] != 208
        || mult3.mat[5] != 246
        || mult3.mat[6] != 284
        || mult3.mat[7] != 322
        || mult3.mat[8] != 112
        || mult3.mat[9] != 134
        || mult3.mat[10] != 156
        || mult3.mat[11] != 178
        || mult3.mat[12] != 16
        || mult3.mat[13] != 22
        || mult3.mat[14] != 28
        || mult3.mat[15] != 34
        ) {
        ++rtn;
    }
    mult1 *= mult2;
    std::cout << "mult1 *= mult2 gives\n" << mult1 << std::endl;
    if (mult1.mat[0] != 304
        || mult1.mat[1] != 358
        || mult1.mat[2] != 412
        || mult1.mat[3] != 466
        || mult1.mat[4] != 208
        || mult1.mat[5] != 246
        || mult1.mat[6] != 284
        || mult1.mat[7] != 322
        || mult1.mat[8] != 112
        || mult1.mat[9] != 134
        || mult1.mat[10] != 156
        || mult1.mat[11] != 178
        || mult1.mat[12] != 16
        || mult1.mat[13] != 22
        || mult1.mat[14] != 28
        || mult1.mat[15] != 34
        ) {
        ++rtn;
    }

    std::array<float, 4> v1 = {1,2,3,4};
    std::array<float, 4> v2;
    std::array<float, 4> v3;
    v2 = mult4 * v1;
    v3 = mult4inv * v2;

    std::cout << "v1 = (" << v1[0]
              << "," << v1[1]
              << "," << v1[2]
              << "," << v1[3] << ")" << std::endl;
    std::cout << "v2 = mult4 * v1 = (" << v2[0]
              << "," << v2[1]
              << "," << v2[2]
              << "," << v2[3] << ")" << std::endl;
    std::cout << "v3 = mult4inv * v2 = (" << v3[0]
              << "," << v3[1]
              << "," << v3[2]
              << "," << v3[3] << ") (should be equal to v1)" << std::endl;

    std::cout << "v1-v3 errors: " << abs(v1[0]-v3[0]) << ", "
              << abs(v1[1]-v3[1]) << ", "
              << abs(v1[2]-v3[2]) << ", "
              << abs(v1[3]-v3[3]) << std::endl;

    float esum = abs(v1[0]-v3[0])
        + abs(v1[1]-v3[1])
        + abs(v1[2]-v3[2])
        + abs(v1[3]-v3[3]);

    if (esum > 1e-5) {
        std::cout << "Inverse failed to re-create the vector" << std::endl;
        ++rtn;
    }

    // test matrix times Vector<T,4> multiplication  std::array = mat * morph::Vector
    morph::Vector<float, 4> v4 = {1,0,0,0};
    std::array<float, 4> r = mult4 * v4;
    std::cout << " mult4 * " << v4 << ": (" << r[0] << "," << r[1] << "," << r[2] << "," << r[3] << ")\n";
    if ((r[0]==15 && r[1]==17 && r[2]==0 && r[3]==0) == false) {
        ++rtn;
    }
#endif
    return rtn;
}
