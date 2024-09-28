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

    morph::Quaternion<float> qz (uz, mc::pi_over_2);

    morph::vec<float> uy_about_z = qz * uy;
    morph::vec<float> ux_about_z = qz * ux;
    morph::vec<float> uz_about_z = qz * uz;

    std::cout << "ux: " << ux << " rotated about the z axis is " << ux_about_z << std::endl;
    std::cout << "uy: " << uy << " rotated about the z axis is " << uy_about_z << std::endl;
    std::cout << "uz: " << uz << " rotated about the z axis is " << uz_about_z << std::endl;

    morph::TransformMatrix<float> tm;
    tm.rotate (qz);

    morph::vec<float, 4> ux_about_z_tm = tm * ux;
    morph::vec<float, 4> uy_about_z_tm = tm * uy;
    morph::vec<float, 4> uz_about_z_tm = tm * uz;

    std::cout << "ux: " << ux << " rotated about the z axis by TM is " << ux_about_z_tm << std::endl;
    std::cout << "uy: " << uy << " rotated about the z axis by TM is " << uy_about_z_tm << std::endl;
    std::cout << "uz: " << uz << " rotated about the z axis by TM is " << uz_about_z_tm << std::endl;

    morph::TransformMatrix<float> tm2;
    tm2.rotate (uz, mc::pi_over_2);

    morph::vec<float, 4> ux_about_z_tm2 = tm2 * ux;
    morph::vec<float, 4> uy_about_z_tm2 = tm2 * uy;
    morph::vec<float, 4> uz_about_z_tm2 = tm2 * uz;

    std::cout << "ux: " << ux << " rotated about the z axis by TM2 is " << ux_about_z_tm2 << std::endl;
    std::cout << "uy: " << uy << " rotated about the z axis by TM2 is " << uy_about_z_tm2 << std::endl;
    std::cout << "uz: " << uz << " rotated about the z axis by TM2 is " << uz_about_z_tm2 << std::endl;
    return 0;
}
