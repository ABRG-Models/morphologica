#include "morph/Quaternion.h"
#include "morph/vec.h"

int main()
{
    int rtn = 0;

    morph::Quaternion<float> q;
    std::cout << q << std::endl;
    q.renormalize();
    std::cout << q << std::endl;

    morph::Quaternion<float> rotationQuaternion;
    float angularSpeed = 0.2;
    morph::vec<float> rotationAxis = {1.0f, 0.0f, 0.0f};
    rotationQuaternion.initFromAxisAngle (rotationAxis, angularSpeed);
    std::cout << rotationQuaternion << std::endl;

    morph::Quaternion<float> p = q;
    if (p == q) {  } else { rtn++; }
    if (p != q) { rtn++; }

    morph::Quaternion<float> qq1 (1.0f, -2.0f, 3.0f, -4.0f);
    std::cout << std::endl << qq1 << " conjugate (q*): " << qq1.conjugate() << std::endl << std::endl;

    morph::Quaternion<float> qq1i = qq1.inverse();
    std::cout << qq1 << " inverse (q^-1 or 1/q): " << qq1i << std::endl << std::endl;

    morph::Quaternion<float> qiqi = qq1i * qq1;
    std::cout << "qq1i * qq1 = " << qiqi << std::endl << std::endl; // 1,0,0,0
    morph::Quaternion<float> qident;
    if (qident == qiqi) {
        std::cout << "Good\n";
    } else {
        --rtn;
        std::cout << "Non good\n";
    }
    std::cout << "cf epsilon: " << std::numeric_limits<float>::epsilon() << std::endl;

    morph::Quaternion<float> q1;
    morph::Quaternion<float> q2;
    using mc = morph::mathconst<float>;
    q1.initFromAxisAngle (morph::vec<float>({1,0,0}), mc::pi_over_3);
    q2.initFromAxisAngle (morph::vec<float>({0,1,0}), mc::pi_over_4);
    morph::Quaternion<float> q3 = q1 * q2;

    std::cout << q3 << " = " << q1 << " * " << q2 << std::endl;
    // q2 *= q1; // no good
    // std::cout << "q2 after *= q1: " << q2 << " cf " << q3 << std::endl;

    // (q2 q1)* = q1* q2*, not q2* q1*
    morph::Quaternion<float> qcpc = q1.conjugate() * q2.conjugate();
    morph::Quaternion<float> pqconj = (q2 * q1).conjugate();
    morph::Quaternion<float> qpconj = (q1 * q2).conjugate();
    if (qcpc == pqconj) {
        std::cout << "Good\n";
    } else {
        --rtn;
    }
    if (qcpc != qpconj) {
        std::cout << "Good\n";
    } else {
        --rtn;
    }
    return rtn;
}
