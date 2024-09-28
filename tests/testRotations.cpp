// Testing rotations with TransformMatrix and Quaternion

#include "morph/Quaternion.h"
#include "morph/TransformMatrix.h"
#include "morph/vec.h"
#include "morph/mathconst.h"

int main()
{
    constexpr morph::vec<float> ux = { 1, 0, 0 };
    constexpr morph::vec<float> uy = { 0, 1, 0 };
    constexpr morph::vec<float> uz = { 0, 0, 1 };

    using mc = morph::mathconst<float>;

    morph::Quaternion<float> qx (ux, mc::pi_over_2);
    morph::vec<float> ux_about_x = qx * ux;
    morph::vec<float> uy_about_x = qx * uy;
    morph::vec<float> uz_about_x = qx * uz;
    std::cout << "ux: " << ux << " rotated about the x axis is " << ux_about_x << std::endl;
    std::cout << "uy: " << uy << " rotated about the x axis is " << uy_about_x << std::endl;
    std::cout << "uz: " << uz << " rotated about the x axis is " << uz_about_x << std::endl;

    morph::Quaternion<float> qy (uy, mc::pi_over_2);
    morph::vec<float> ux_about_y = qy * ux;
    morph::vec<float> uy_about_y = qy * uy;
    morph::vec<float> uz_about_y = qy * uz;
    std::cout << std::endl
              << "ux: " << ux << " rotated about the y axis is " << ux_about_y << std::endl;
    std::cout << "uy: " << uy << " rotated about the y axis is " << uy_about_y << std::endl;
    std::cout << "uz: " << uz << " rotated about the y axis is " << uz_about_y << std::endl;

    morph::Quaternion<float> qz (uz, mc::pi_over_2);
    morph::vec<float> ux_about_z = qz * ux;
    morph::vec<float> uy_about_z = qz * uy;
    morph::vec<float> uz_about_z = qz * uz;
    std::cout << std::endl
              << "ux: " << ux << " rotated about the z axis is " << ux_about_z << std::endl;
    std::cout << "uy: " << uy << " rotated about the z axis is " << uy_about_z << std::endl;
    std::cout << "uz: " << uz << " rotated about the z axis is " << uz_about_z << std::endl;

    std::cout << "\n\n";

    morph::TransformMatrix<float> tmx;
    tmx.rotate (qx);
    morph::vec<float, 4> ux_about_tmx = tmx * ux;
    morph::vec<float, 4> uy_about_tmx = tmx * uy;
    morph::vec<float, 4> uz_about_tmx = tmx * uz;
    std::cout << "ux: " << ux << " rotated about the x axis by TM is " << ux_about_tmx << std::endl;
    std::cout << "uy: " << uy << " rotated about the x axis by TM is " << uy_about_tmx << std::endl;
    std::cout << "uz: " << uz << " rotated about the x axis by TM is " << uz_about_tmx << std::endl;

    morph::TransformMatrix<float> tmy;
    tmy.rotate (qy);
    morph::vec<float, 4> ux_about_tmy = tmy * ux;
    morph::vec<float, 4> uy_about_tmy = tmy * uy;
    morph::vec<float, 4> uz_about_tmy = tmy * uz;
    std::cout << std::endl
              << "ux: " << ux << " rotated about the y axis by TM is " << ux_about_tmy << std::endl;
    std::cout << "uy: " << uy << " rotated about the y axis by TM is " << uy_about_tmy << std::endl;
    std::cout << "uz: " << uz << " rotated about the y axis by TM is " << uz_about_tmy << std::endl;

    morph::TransformMatrix<float> tmz;
    tmz.rotate (qz);
    morph::vec<float, 4> ux_about_tmz = tmz * ux;
    morph::vec<float, 4> uy_about_tmz = tmz * uy;
    morph::vec<float, 4> uz_about_tmz = tmz * uz;
    std::cout << std::endl
              << "ux: " << ux << " rotated about the z axis by TM is " << ux_about_tmz << std::endl;
    std::cout << "uy: " << uy << " rotated about the z axis by TM is " << uy_about_tmz << std::endl;
    std::cout << "uz: " << uz << " rotated about the z axis by TM is " << uz_about_tmz << std::endl;


    return 0;
}
