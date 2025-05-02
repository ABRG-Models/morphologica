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

    // Translation of [1,0,0], then the rotation 90 deg around z axis

    morph::vec<F> ux_about_z_truth_pretrans = { 0.0, 2.0, 0.0 };
    morph::vec<F> uy_about_z_truth_pretrans = {-1.0, 1.0, 0.0 };
    morph::vec<F> uz_about_z_truth_pretrans = { 0.0, 1.0, 1.0 };

    morph::quaternion<F> qz (uz, mc::pi_over_2);

    morph::mat44<F> tmz_pt;
    tmz_pt.rotate (qz);
    tmz_pt.pretranslate (ux);

    std::cout << "Linear part returned : " << tmz_pt.linear() << std::endl << std::endl;
    std::cout << "Translation part returned : " << tmz_pt.translation() << std::endl << std::endl;

    morph::vec<F, 4> ux_about_tmz_pt = tmz_pt * ux;
    morph::vec<F, 4> uy_about_tmz_pt = tmz_pt * uy;
    morph::vec<F, 4> uz_about_tmz_pt = tmz_pt * uz;

    std::cout << std::endl
              << "ux: " << ux << " rotated about the z axis and pre-translated by ux using TM is " << ux_about_tmz_pt << "\nTRUTH : " << ux_about_z_truth_pretrans << std::endl << std::endl;
    std::cout << "uy: " << uy << " rotated about the z axis and pre-translated by ux using TM is " << uy_about_tmz_pt << "\nTRUTH : " << uy_about_z_truth_pretrans << std::endl << std::endl;
    std::cout << "uz: " << uz << " rotated about the z axis and pre-translated by ux using TM is " << uz_about_tmz_pt << "\nTRUTH : " << uz_about_z_truth_pretrans << std::endl << std::endl;

    // Had to scale the epsilon here because the pretranslate pushes the values further from 1.
    if    ((ux_about_tmz_pt.less_one_dim() - ux_about_z_truth_pretrans).abs().max() > 2.0 * std::numeric_limits<F>::epsilon()
        || (uy_about_tmz_pt.less_one_dim() - uy_about_z_truth_pretrans).abs().max() > 2.0 * std::numeric_limits<F>::epsilon()
        || (uz_about_tmz_pt.less_one_dim() - uz_about_z_truth_pretrans).abs().max() > 2.0 * std::numeric_limits<F>::epsilon()) { --rtn; }

    // Alternative ordering. pretranslate first, then rotate should give the same result
    morph::mat44<F> tmz_pt2;
    tmz_pt2.pretranslate (ux);
    tmz_pt2.rotate (qz);

    // Translate first then rotate should also give the same result
    morph::mat44<F> tmz_pt3;
    tmz_pt3.translate (ux);
    tmz_pt3.rotate (qz);

    morph::vec<F, 4> ux_about_tmz_pt2 = tmz_pt2 * ux;
    std::cout << "tmz_pt2 * ux = " << ux_about_tmz_pt2<< " cf. tmz_pt * ux = " << ux_about_tmz_pt << std::endl;
    std::cout << "tmz_pt3 * ux = " << (tmz_pt3 * ux) << " cf. tmz_pt * ux = " << ux_about_tmz_pt << std::endl;

    if ((tmz_pt3 * ux) != (tmz_pt2 * ux) || (tmz_pt2 * ux) != (tmz_pt * ux)) {
        --rtn;
    }

    // Translation of [0,1,0], then the rotation -90 deg around y axis

    morph::vec<F> ux_about_y_truth_pretrans = { 0.0, 1.0, -1.0 };
    morph::vec<F> uy_about_y_truth_pretrans = { 0.0, 2.0, 0.0 };
    morph::vec<F> uz_about_y_truth_pretrans = { 1.0, 1.0, 0.0 };

    morph::quaternion<F> qy (uy, mc::pi_over_2);

    morph::mat44<F> tmy_pt;
    tmy_pt.rotate (qy);
    tmy_pt.pretranslate (uy);

    morph::vec<F, 4> ux_about_tmy_pt = tmy_pt * ux;
    morph::vec<F, 4> uy_about_tmy_pt = tmy_pt * uy;
    morph::vec<F, 4> uz_about_tmy_pt = tmy_pt * uz;

    std::cout << std::endl
              << "ux: " << ux << " rotated about the y axis and pre-translated by uy using TM is " << ux_about_tmy_pt << "\nTRUTH : " << ux_about_y_truth_pretrans << std::endl << std::endl;
    std::cout << "uy: " << uy << " rotated about the y axis and pre-translated by uy using TM is " << uy_about_tmy_pt << "\nTRUTH : " << uy_about_y_truth_pretrans << std::endl << std::endl;
    std::cout << "uz: " << uz << " rotated about the y axis and pre-translated by uy using TM is " << uz_about_tmy_pt << "\nTRUTH : " << uz_about_y_truth_pretrans << std::endl << std::endl;

    // HAd to scale the epsilon here because the pretranslate pushes the values further from 1.
    if    ((ux_about_tmy_pt.less_one_dim() - ux_about_y_truth_pretrans).abs().max() > 2.0 * std::numeric_limits<F>::epsilon()
        || (uy_about_tmy_pt.less_one_dim() - uy_about_y_truth_pretrans).abs().max() > 2.0 * std::numeric_limits<F>::epsilon()
        || (uz_about_tmy_pt.less_one_dim() - uz_about_y_truth_pretrans).abs().max() > 2.0 * std::numeric_limits<F>::epsilon()) { --rtn; }


    if (rtn == 0) {
        std::cout << "Pretranslation tests PASSED\n";
    } else {
        std::cout << "Pretranslation tests FAILED\n";
    }

    return rtn;
}
