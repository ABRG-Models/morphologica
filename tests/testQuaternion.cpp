#include "morph/Quaternion.h"
#include "morph/vec.h"
#include "morph/mathconst.h"

int main()
{
    int rtn = 0;

    morph::Quaternion<float> q;
    std::cout << q << std::endl;
    morph::Quaternion<float> q_save = q;
    q.renormalize();
    std::cout << q << std::endl;

    if (q != q_save) { rtn++; }

    float angularSpeed = 0.2 * morph::mathconst<float>::deg2rad;
    morph::vec<float> rotationAxis = {1.0f, 0.0f, 0.0f};
    morph::Quaternion<float> rotationQuaternion(rotationAxis, angularSpeed);
    morph::Quaternion<float> rq_expected (float{1}, angularSpeed/float{2}, float{0}, float{0});
    std::cout << "Quaternion(" << rotationAxis<< ", " << angularSpeed<< ") constructor generates\n"
              << rotationQuaternion
              << "\nvs expected:\n" << rq_expected
              << std::endl;
    morph::vec<float, 4> rqerr;
    rqerr[0] = rq_expected.x - rotationQuaternion.x;
    rqerr[1] = rq_expected.y - rotationQuaternion.y;
    rqerr[2] = rq_expected.z - rotationQuaternion.z;
    rqerr[3] = rq_expected.w - rotationQuaternion.w;
    if (rqerr.abs().max() > float{15} * std::numeric_limits<float>::epsilon()) {
        std::cout << "Failed on rotation. Errors: " << rqerr << " cf epsilon: "
                  << std::numeric_limits<float>::epsilon() << std::endl;
        rtn++;
    }


    morph::Quaternion<float> p = q;
    if (p == q) {  } else { rtn++; }
    if (p != q) { rtn++; }

    morph::Quaternion<float> qq1 (1.0f, -2.0f, 3.0f, -4.0f);
    std::cout << std::endl << qq1 << " conjugate (q*): " << qq1.conjugate() << std::endl << std::endl;
    morph::Quaternion<float> qq1conj (1.0f, 2.0f, -3.0f, 4.0f);
    if (qq1.conjugate() != qq1conj) { ++rtn; }


    morph::Quaternion<float> qq1i = qq1.inverse();
    std::cout << qq1 << " inverse (q^-1 or 1/q): " << qq1i << std::endl << std::endl;

    morph::Quaternion<float> qq2 (1.0f, 0.0f, 0.0f, 0.0f);
    morph::Quaternion<float> qq2i = qq2.inverse();
    std::cout << qq2 << " inverse (q^-1 or 1/q): " << qq2i << std::endl << std::endl;

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

    using mc = morph::mathconst<float>;
    morph::Quaternion<float> q1(morph::vec<float>({1,0,0}), mc::pi_over_3);
    morph::Quaternion<float> q2(morph::vec<float>({0,1,0}), mc::pi_over_4);
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

    // Rotation matrices
    morph::Quaternion<float> qfm (1, 2, -3, 4); // NOT unit.
    std::array<float, 16> matA = qfm.rotationMatrix();
    std::array<float, 16> matB = qfm.unitRotationMatrix();
    //std::array<float, 16> matC;
    //qfm.rotationMatrix2(matC);
    morph::vec<float, 16> vmatA;
    vmatA.set_from (matA);
    morph::vec<float, 16> vmatB;
    vmatB.set_from (matB);
    //morph::vec<float, 16> vmatC;
    //vmatC.set_from (matC);
    std::cout << "Rotation matrices of non-unit qfm\n";
    std::cout << "rotationMatrix:     " << vmatA << std::endl;
    //std::cout << "unitRotationMatrix2: " << vmatC << std::endl;
    std::cout << "unitRotationMatrix: " << vmatB << std::endl;


    std::cout << "Rotation matrices of unit qfm\n";
    qfm.renormalize();
    std::array<float, 16> matAA = qfm.rotationMatrix();
    std::array<float, 16> matBB = qfm.unitRotationMatrix();
    //std::array<float, 16> matCC;
    //qfm.rotationMatrix2(matCC);
    morph::vec<float, 16> vmatAA;
    vmatAA.set_from (matAA);
    morph::vec<float, 16> vmatBB;
    vmatBB.set_from (matBB);
    //morph::vec<float, 16> vmatCC;
    //vmatCC.set_from (matCC);
    std::cout << "rotationMatrix:     " << vmatAA << std::endl;
    //std::cout << "unitRotationMatrix2: " << vmatCC << std::endl;
    std::cout << "unitRotationMatrix: " << vmatBB << std::endl;



    if (rtn == 0) {
        std::cout << "Quaternion tests PASSED\n";
    } else {
        std::cout << "Quaternion tests FAILED\n";
    }

    return rtn;
}
