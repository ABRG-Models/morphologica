#include "morph/quaternion.h"
#include "morph/vec.h"
#include "morph/mathconst.h"
#include "morph/Random.h"

int main()
{
    int rtn = 0;

    morph::quaternion<float> q;
    std::cout << q << std::endl;
    morph::quaternion<float> q_save = q;
    q.renormalize();
    std::cout << q << std::endl;

    if (q != q_save) { rtn++; }

    float angularSpeed = 0.2 * morph::mathconst<float>::deg2rad;
    morph::vec<float> rotationAxis = {1.0f, 0.0f, 0.0f};
    morph::quaternion<float> rotationquaternion(rotationAxis, angularSpeed);
    morph::quaternion<float> rq_expected (float{1}, angularSpeed/float{2}, float{0}, float{0});
    std::cout << "quaternion(" << rotationAxis<< ", " << angularSpeed<< ") constructor generates\n"
              << rotationquaternion
              << "\nvs expected:\n" << rq_expected
              << std::endl;
    morph::vec<float, 4> rqerr;
    rqerr[0] = rq_expected.x - rotationquaternion.x;
    rqerr[1] = rq_expected.y - rotationquaternion.y;
    rqerr[2] = rq_expected.z - rotationquaternion.z;
    rqerr[3] = rq_expected.w - rotationquaternion.w;
    if (rqerr.abs().max() > float{15} * std::numeric_limits<float>::epsilon()) {
        std::cout << "Failed on rotation. Errors: " << rqerr << " cf epsilon: "
                  << std::numeric_limits<float>::epsilon() << std::endl;
        rtn++;
    }


    morph::quaternion<float> p = q;
    if (p == q) {  } else { rtn++; }
    if (p != q) { rtn++; }

    morph::quaternion<float> qq1 (1.0f, -2.0f, 3.0f, -4.0f);
    std::cout << std::endl << qq1 << " conjugate (q*): " << qq1.conjugate() << std::endl << std::endl;
    morph::quaternion<float> qq1conj (1.0f, 2.0f, -3.0f, 4.0f);
    if (qq1.conjugate() != qq1conj) { ++rtn; }


    morph::quaternion<float> qq1i = qq1.inverse();
    std::cout << qq1 << " inverse (q^-1 or 1/q): " << qq1i << std::endl << std::endl;

    morph::quaternion<float> qq2 (1.0f, 0.0f, 0.0f, 0.0f);
    morph::quaternion<float> qq2i = qq2.inverse();
    std::cout << qq2 << " inverse (q^-1 or 1/q): " << qq2i << std::endl << std::endl;

    morph::quaternion<float> qiqi = qq1i * qq1;
    std::cout << "qq1i * qq1 = " << qiqi << std::endl << std::endl; // 1,0,0,0
    morph::quaternion<float> qident;
    if (qident == qiqi) {
        std::cout << "Good\n";
    } else {
        --rtn;
        std::cout << "Non good\n";
    }
    std::cout << "cf epsilon: " << std::numeric_limits<float>::epsilon() << std::endl;

    using mc = morph::mathconst<float>;
    morph::quaternion<float> q1(morph::vec<float>({1,0,0}), mc::pi_over_3);
    morph::quaternion<float> q2(morph::vec<float>({0,1,0}), mc::pi_over_4);
    morph::quaternion<float> q3 = q1 * q2;

    std::cout << q3 << " = " << q1 << " * " << q2 << std::endl;
    // q2 *= q1; // no good
    // std::cout << "q2 after *= q1: " << q2 << " cf " << q3 << std::endl;

    // (q2 q1)* = q1* q2*, not q2* q1*
    morph::quaternion<float> qcpc = q1.conjugate() * q2.conjugate();
    morph::quaternion<float> pqconj = (q2 * q1).conjugate();
    morph::quaternion<float> qpconj = (q1 * q2).conjugate();
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
    morph::quaternion<float> qfm (1, 2, -3, 4); // NOT unit.
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


    morph::vec<float> myaxis = { 1, 2, 3 };
    float myangle = 0.12f;
    myaxis.renormalize();
    morph::quaternion<float> qaa (myaxis, myangle);
    morph::vec<float, 4> aa = qaa.axis_angle();

    std::cout << "\nquaternion " << qaa << "\n"
              << "was set from rotn " << myangle << " about axis " << myaxis << "\n"
              << "and its axis_angle method returns " << aa << "\n"
              << "which is a rotation of " << aa[3] << " rads about axis " << aa.less_one_dim() << std::endl << std::endl;

    float eps = 0.00001; // A fair amount of precision is lost extracting axis angle if type is float
    if (std::abs(aa[0] - myaxis[0]) > eps
        || std::abs(aa[1] - myaxis[1]) > eps
        || std::abs(aa[2] - myaxis[2]) > eps
        || std::abs(aa[3] - myangle) > eps) {
        --rtn;
    }

    morph::quaternion<float> q_unitf (1.0f, 2.0f, -3.0f, 4.0f); // NOT unit.
    q_unitf.renormalize();
    morph::RandUniform<float> rngf;
    morph::vec<float> vecf;
    float amountf;
    morph::range<float> metric_rangef (0.0f, 0.0f);
    for (unsigned int i = 0; i < 1000000; ++i) {
        rngf.get(vecf);
        vecf.renormalize();
        amountf = rngf.get();
        q_unitf.rotate (vecf, amountf);
        float metricf = morph::math::abs(float{1} - (q_unitf.w*q_unitf.w + q_unitf.x*q_unitf.x + q_unitf.y*q_unitf.y + q_unitf.z*q_unitf.z));
        // how big is metric? Should be about 0.
        metric_rangef.update (metricf);
    }
    std::cout << "metric_range (float): " << metric_rangef << std::endl;
    // metric_range should be smaller than unitThresh
    if (metric_rangef.max > morph::quaternion<float>::unitThresh()) {
        --rtn;
    }

    morph::quaternion<double> q_unitd (1.0, 2.0, -3.0, 4.0); // NOT unit.
    q_unitd.renormalize();
    morph::RandUniform<double> rngd;
    morph::vec<double> vecd;
    double amountd;
    morph::range<double> metric_ranged (0.0f, 0.0f);
    for (unsigned int i = 0; i < 1000000; ++i) {
        rngd.get(vecd);
        vecd.renormalize();
        amountd = rngd.get();
        q_unitd.rotate (vecd, amountd);
        double metricd = morph::math::abs(double{1} - (q_unitd.w*q_unitd.w + q_unitd.x*q_unitd.x + q_unitd.y*q_unitd.y + q_unitd.z*q_unitd.z));
        // how big is metric? Should be about 0.
        metric_ranged.update (metricd);
    }
    std::cout << "metric_range (double): " << metric_ranged << std::endl;
    // metric_range should be smaller than unitThresh
    if (metric_ranged.max > morph::quaternion<double>::unitThresh()) {
        --rtn;
    }

    if (rtn == 0) {
        std::cout << "quaternion tests PASSED\n";
    } else {
        std::cout << "quaternion tests FAILED\n";
    }

    return rtn;
}
