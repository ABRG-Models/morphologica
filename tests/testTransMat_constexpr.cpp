#include "morph/mat44.h"
#include "morph/quaternion.h"
#include <iostream>
#include <array>
#include <morph/vec.h>
#include <morph/constexpr_math.h>
#include <cmath>

constexpr void setMatrixSequence (morph::mat44<float>& tm)
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
    tm.mat[9] = 9;
    tm.mat[10] = 10;
    tm.mat[11] = 11;
    tm.mat[12] = 12;
    tm.mat[13] = 13;
    tm.mat[14] = 14;
    tm.mat[15] = 15;
}

constexpr int do_test()
{
    int rtn = 0;

    // Test assignment
    morph::mat44<float> tm1;
    setMatrixSequence (tm1);

    morph::mat44<float> tm2 = tm1;
    for (unsigned int i = 0; i<16; ++i) {
        if (tm2.mat[i] != (float)i) {
            ++rtn;
        }
    }

    tm2 = tm1;
    for (unsigned int i = 0; i<16; ++i) {
        if (tm2.mat[i] != (float)i) {
            ++rtn;
        }
    }

    // Test multiplication
    morph::mat44<float> mult1;
    setMatrixSequence (mult1);

    morph::mat44<float> mult2;
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

    morph::mat44<float> mult3 = mult1 * mult2;

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

    // Test 3x3 determinant
    morph::mat44<float> td;
    std::array<float, 9> threethree = { 1.0f, 0.0f, 2.0f, 1.0f, 1.0f, 3.5f, 3.0f, 2.0f, 120.0f };
    float det_td = td.determinant3x3 (threethree);
    if (det_td != 111.0f) {
        ++rtn;
    }

    // Test 4x4 determinant
    std::array<float, 16> fourfour = { 2.0f, 7.0f, 5.0f, 6.0f, 8.0f, 1.0f, 3.0f, 6.0f, 2.0f, 8.0f, -1.0f, 7.0f, 7.0f, 0.0f, 1.0f, 7.0f };
    float det_td2 = td.determinant (fourfour);
    if (det_td2 != 816.0f) {
        ++rtn;
    }

    // Test matrix inversion
    morph::mat44<float> mult4;
    mult4.mat[0] = 15;
    mult4.mat[1] = 17;
    mult4.mat[2] = 0;
    mult4.mat[3] = 0;
    mult4.mat[4] = 2;
    mult4.mat[5] = 10;
    mult4.mat[6] = 0;
    mult4.mat[7] = 0;
    mult4.mat[8] = 0;
    mult4.mat[9] = 0;
    mult4.mat[10] = 5;
    mult4.mat[11] = 4;
    mult4.mat[12] = 0;
    mult4.mat[13] = 0;
    mult4.mat[14] = 1;
    mult4.mat[15] = 0;

    morph::mat44<float> mult4inv = mult4.invert();

    std::array<float, 4> v1 = {1,2,3,4};
    std::array<float, 4> v2;
    std::array<float, 4> v3;
    v2 = mult4 * v1;
    v3 = mult4inv * v2;

    float esum = morph::math::abs(v1[0]-v3[0])
        + morph::math::abs(v1[1]-v3[1])
        + morph::math::abs(v1[2]-v3[2])
        + morph::math::abs(v1[3]-v3[3]);

    if (esum > 1e-5) {
        //std::cout << "Inverse failed to re-create the vector" << std::endl;
        ++rtn;
    }

    // test matrix times vec<T,4> multiplication  std::array = mat * morph::vec
    morph::vec<float, 4> v4 = {1,0,0,0};
    std::array<float, 4> r = mult4 * v4;
    if ((r[0]==15 && r[1]==17 && r[2]==0 && r[3]==0) == false) {
        ++rtn;
    }

    morph::mat44<float> mult4inv_copy = mult4inv;
    if (mult4inv_copy != mult4inv) { ++rtn; }

    mult4inv_copy.setToIdentity();
    if (mult4inv_copy[0] != 1.0f) { ++rtn; }
    morph::vec<float, 4> r0 = mult4inv_copy.row(0);
    if (r0[0] != 1.0f) { ++rtn; }
    morph::vec<float, 4> c0 = mult4inv_copy.col(0);
    if (c0[0] != 1.0f) { ++rtn; }

    mult4inv_copy.translate (morph::vec<float, 3>{1, 0, 0});
    mult4inv_copy.translate (std::array<float, 3>{-1, 0, 0});
    mult4inv_copy.translate (0.0f, 0.0f, 0.0f);
    if (mult4inv_copy[0] != 1.0f) { ++rtn; }

    mult4inv_copy.perspective (25.0f, 2.0f, 0.1f, 10.0f);
    const morph::vec<float, 2> lb = { -4, -5 };
    const morph::vec<float, 2>& rt = { 4, 5 };
    mult4inv_copy.orthographic (lb, rt, 0.1f, 10.0f);
    mult4inv_copy.setToIdentity();
    if (mult4inv_copy[0] != 1.0f) { ++rtn; }

    morph::quaternion<float> q_f;
    morph::quaternion<double> q_d;
    mult4inv_copy.rotate (q_f);
    mult4inv_copy.rotate (q_d);
    mult4inv_copy.setToIdentity();
    if (mult4inv_copy[0] != 1.0f) { ++rtn; }

    return rtn;
}

int main()
{
    constexpr int rtn = do_test();
    if constexpr (rtn == 0) { std::cout << "Success\n"; }
    return rtn;
}
