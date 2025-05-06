// Testing rotations of unit vectors about unit axes with mat44 multiplication
// and quaternion multiplication

#include "morph/quaternion.h"
#include "morph/mat44.h"
#include "morph/vec.h"
#include "morph/mathconst.h"

#ifndef FLT
typedef float F;
#else
typedef FLT   F;
#endif

int main()
{
    int rtn = 0;

    constexpr morph::vec<F> ux = { 1, 0, 0 };
    constexpr morph::vec<F> uy = { 0, 1, 0 };
    constexpr morph::vec<F> uz = { 0, 0, 1 };

    using mc = morph::mathconst<F>;

    // Rotation of 90 around Z axis+ Translation of [1,0,0], then a PRE rotation 90 deg around X axis

    morph::vec<F> ux_truth_prerotate = { 1.0, 1.0, 0.0 };
    morph::vec<F> uy_truth_prerotate = { 1.0, 0.0, 1.0 };
    morph::vec<F> uz_truth_prerotate = { 2.0, 0.0, 0.0 };

    morph::quaternion<F> qx (ux, mc::pi_over_2);
    morph::quaternion<F> qz (uz, mc::pi_over_2);

    morph::mat44<F> tm_pr;
    tm_pr.rotate (qz);
    tm_pr.translate (ux);
    tm_pr.prerotate (qx);

    morph::vec<F, 4> ux_about_tm_pr = tm_pr * ux;
    morph::vec<F, 4> uy_about_tm_pr = tm_pr * uy;
    morph::vec<F, 4> uz_about_tm_pr = tm_pr * uz;

    std::cout << std::endl
              << "ux: " << ux << ",  Rotation of 90 around Z axis + Translation of [1,0,0], then a PRE rotation 90 deg around X axis  " << ux_about_tm_pr << "\nTRUTH : " << ux_truth_prerotate << std::endl << std::endl;
    std::cout << "uy: " << uy << ",  Rotation of 90 around Z axis + Translation of [1,0,0], then a PRE rotation 90 deg around X axis  " << uy_about_tm_pr << "\nTRUTH : " << uy_truth_prerotate << std::endl << std::endl;
    std::cout << "uz: " << uz << ",  Rotation of 90 around Z axis + Translation of [1,0,0], then a PRE rotation 90 deg around X axis  " << uz_about_tm_pr << "\nTRUTH : " << uz_truth_prerotate << std::endl << std::endl;

    // Had to scale the epsilon here because the pretranslate pushes the values further from 1.
    if    ((ux_about_tm_pr.less_one_dim() - ux_truth_prerotate).abs().max() > 2.0 * std::numeric_limits<F>::epsilon()
        || (uy_about_tm_pr.less_one_dim() - uy_truth_prerotate).abs().max() > 2.0 * std::numeric_limits<F>::epsilon()
        || (uz_about_tm_pr.less_one_dim() - uz_truth_prerotate).abs().max() > 2.0 * std::numeric_limits<F>::epsilon()) { --rtn; }

    if (rtn == 0) {
        std::cout << "Prerotations tests PASSED\n";
    } else {
        std::cout << "Prerotations tests FAILED\n";
    }

    return rtn;
}