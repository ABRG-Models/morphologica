#include <iostream>
#include "morph/Quaternion.h"
#include "morph/vec.h"

constexpr int test_quat1()
{
    int rtn = 0;

    morph::Quaternion<float> q;
    q.renormalize();

    morph::Quaternion<float> rotationQuaternion;
    float angularSpeed = 0.2;
    morph::vec<float> rotationAxis = {1.0f, 0.0f, 0.0f};
    rotationQuaternion.initFromAxisAngle (rotationAxis, angularSpeed);

    morph::Quaternion<float> p = q;
    if (p == q) {  } else { rtn++; }
    if (p != q) { rtn++; }

    morph::Quaternion<float> qq1 (1.0f, -2.0f, 3.0f, -4.0f);

    morph::Quaternion<float> qq1i = qq1.inverse();

    morph::Quaternion<float> qiqi = qq1i * qq1;
    morph::Quaternion<float> qident;
    if (qident != qiqi) { --rtn; }

    morph::Quaternion<float> q1;
    morph::Quaternion<float> q2;
    using mc = morph::mathconst<float>;
    q1.initFromAxisAngle (morph::vec<float>({1,0,0}), mc::pi_over_3);
    q2.initFromAxisAngle (morph::vec<float>({0,1,0}), mc::pi_over_4);
    morph::Quaternion<float> q3 = q1 * q2;
    if (q3.x == 0) { q3.x += 0.1f; }


    // (q2 q1)* = q1* q2*, not q2* q1*
    morph::Quaternion<float> qcpc = q1.conjugate() * q2.conjugate();
    morph::Quaternion<float> pqconj = (q2 * q1).conjugate();
    morph::Quaternion<float> qpconj = (q1 * q2).conjugate();
    if (qcpc != pqconj) { --rtn; }
    if (qcpc == qpconj) { --rtn; }

    if (qident.magnitude() != 1.0f) { --rtn; }
    qident.reset();
    q1.postmultiply(q2);
    q2.premultiply(qident);

    q2.rotate (1.0f, 0.0f, 0.0f, mc::pi_over_2);
    q2.rotate (std::array<float, 3>{1.0f, 0.0f, 0.0f}, mc::pi_over_2);
    q2.rotate (morph::vec<float, 3>{1.0f, 0.0f, 0.0f}, mc::pi_over_2);

    std::array<float, 16> am = q1.rotationMatrix();
    if (am[0] != 1.0f) { am[0] += 1.0f; } // Just avoid unused variable for am

    std::array<float, 16> am2 = q2.unitRotationMatrix();
    if (am2[0] != 1.0f) { am2[0] += 1.0f; } // Just avoid unused variable for am

    morph::Quaternion<float> qinvert = q1.invert();
    if (qinvert.x == 0) { qinvert.x += 1; }
    return rtn;
}

int main()
{
    constexpr int rtn = test_quat1();
    if constexpr (rtn == 0) { std::cout << "Success\n"; }
    return rtn;
}
