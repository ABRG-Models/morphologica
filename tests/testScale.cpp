#include <limits>
#include <vector>
#include <list>
#include <array>
#include <iostream>
#include "morph/scale.h"
#include <cmath>

int main ()
{
    int rtn = 0;

    char testc = 127;
    std::cout << "test char" << (int)testc << std::endl;
    testc += 1;
    std::cout << "test char+1" << (int)testc << std::endl;

    std::cout << "char max" << std::numeric_limits<char>::max() << std::endl;
    std::cout << "unsigned char max" << std::numeric_limits<unsigned char>::max() << std::endl;
    std::cout << "unsigned short max" << std::numeric_limits<unsigned short>::max() << std::endl;

    morph::scale<float> s;
    s.do_autoscale = true;
    std::vector<float> vf = {1,2,3,4,5,8,9,18};
    std::vector<float> result(vf);
    s.transform (vf, result);
    std::cout << "s output_range: " << s.output_range << std::endl;
    std::cout << "1st data: Unscaled/scaled: ";
    for (unsigned int i = 0; i < vf.size(); ++i) {
        std::cout << vf[i]<<"/"<<result[i]<<", ";
    }
    std::cout << std::endl;

    // Test scalar scaling
    if (std::abs(result.back() - 1.0f) > std::numeric_limits<float>::epsilon()) {
        std::cout << "Error in scalar scaling" << std::endl;
        rtn--;
    }
    if (std::abs(result.front() - 0.0f) > std::numeric_limits<float>::epsilon()) {
        std::cout << "Error in scalar scaling" << std::endl;
        rtn--;
    }

    // Test different output range ([1,2] instead of the default [0,1])
    morph::scale<float> s_2;
    s_2.do_autoscale = true;
    s_2.output_range.min = 1.0f;
    s_2.output_range.max = 2.0f;
    std::vector<float> result_2(vf);
    s_2.transform (vf, result_2);
    std::cout << "New range: Unscaled / scaled [0,1]  / scaled [1,2]\n";
    for (unsigned int i = 0; i < vf.size(); ++i) {
        std::cout << vf[i] << " / " << result[i] << " / " << result_2[i] << "\n";
    }
    std::cout << std::endl;

    // Different data, but extend max a bit. The result should now span >0,1
    // range. This shows that the autoscaling is carried out once only by the scale
    // object. To autoscale again with vf2, call s.reset()
    std::vector<float> vf2 = {1,2,3,4,5,8,9,32};
    //s.reset()   // would force re-autoscale when transform(vf2) next called
    //s.autoscale_from (vf2); // will immediately autoscale from vf2.
    s.transform (vf2, result);
    std::cout << "2nd data: Unscaled/scaled: ";
    for (unsigned int i = 0; i < vf2.size(); ++i) {
        std::cout << vf2[i]<<"/"<<result[i]<<", ";
    }
    std::cout << std::endl;

    std::cout << "Integer to float scaling:\n";
    morph::scale<int,float> si;
    si.do_autoscale = true;
    std::vector<int> vfi = {-19,1,2,3,4,5,8,9,18};
    std::vector<float> resulti(vfi.size());
    si.transform (vfi, resulti);
    std::cout << "1st data: Unscaled/scaled: ";
    for (unsigned int i = 0; i < vfi.size(); ++i) {
        std::cout << vfi[i]<<"/"<<resulti[i]<<", ";
    }
    std::cout << std::endl;
    std::cout << "Stream scale<int, float>: " << si << std::endl;
    // Test integer scalar scaling
    if (std::abs(resulti.back() - 1.0f) > std::numeric_limits<float>::epsilon()) {
        std::cout << "Error in integer scalar scaling" << std::endl;
        rtn--;
    }
    if (std::abs(resulti.front() - 0.0f) > std::numeric_limits<float>::epsilon()) {
        std::cout << "Error in integer scalar scaling" << std::endl;
        rtn--;
    }

    std::cout << "unsigned char to float scaling:\n";
    morph::scale<unsigned char,float> suc;
    suc.do_autoscale = true;
    std::vector<unsigned char> vfuc = {1,2,3,4,5,8,9,18};
    std::vector<float> resultuc(vfuc.size());
    suc.transform (vfuc, resultuc);
    std::cout << "1st data: Unscaled/scaled: ";
    for (unsigned int i = 0; i < vfuc.size(); ++i) {
        std::cout << (unsigned int)vfuc[i]<<"/"<<resultuc[i]<<", ";
    }
    std::cout << std::endl;
    // Test unsigned char to float scalar scaling
    if (std::abs(resultuc.back() - 1.0f) > std::numeric_limits<float>::epsilon()) {
        std::cout << "Error in unsigned char scalar scaling" << std::endl;
        rtn--;
    }
    if (std::abs(resultuc.front() - 0.0f) > std::numeric_limits<float>::epsilon()) {
        std::cout << "Error in unsigned char scalar scaling" << std::endl;
        rtn--;
    }

    morph::scale<std::array<float,4>> s2;
    s2.do_autoscale = true;
    std::vector<std::array<float,4>> vaf;
    vaf.push_back ({1,1,2,1});
    vaf.push_back ({2,2,2,3});
    vaf.push_back ({3,3,4,1});
    vaf.push_back ({4,4,4,4});
    std::vector<std::array<float,4>> result2(vaf);
    s2.transform (vaf, result2);

    std::cout << "vector<array<float,4>> unscaled/scaled vectors:\n";
    for (unsigned int i = 0; i < result2.size(); ++i) {

        std::cout << "(";
        for (auto v : vaf[i]) {
            std::cout << v << ",";
        }
        std::cout << ")   ";

        std::cout << "(";
        for (auto v : result2[i]) {
            std::cout << v << ",";
        }
        std::cout << ")\n";
    }

    // Test this scaling:
    std::vector<std::array<float, 4>>::const_iterator r2i = result2.end();
    r2i--; // To get to last element in vector
    float r2ilen = std::sqrt ((*r2i)[0] * (*r2i)[0] + (*r2i)[1] * (*r2i)[1] + (*r2i)[2] * (*r2i)[2] + (*r2i)[3] * (*r2i)[3]);
    if (std::abs(r2ilen - 1) > 0.0001) {
        std::cout << "Error" << std::endl;
        rtn--;
    }

    morph::scale<std::vector<double>> s3;
    s3.do_autoscale = true;
    std::list<std::vector<double>> vaf3;
    vaf3.push_back ({1,1,1});
    vaf3.push_back ({2,2,2});
    vaf3.push_back ({3,3,3});
    vaf3.push_back ({4,4,4});
    std::list<std::vector<double>> result3(vaf3);
    s3.transform (vaf3, result3);

    std::cout << "list<vector<double>> scaled vectors:\n";
    std::list<std::vector<double>>::iterator res3i = result3.begin();
    while (res3i != result3.end()) {
        std::cout << "(";
        std::vector<double>::iterator vi = res3i->begin();
        while (vi != res3i->end()) {
            std::cout << *vi++ << ",";
        }
        std::cout << ")\n";
        ++res3i;
    }

    // Log scaling
    std::cout << "Log scaling...\n";
    morph::scale<double, float> ls;
    ls.do_autoscale = true;
    ls.setlog();

    morph::vvec<double> loggy;
    loggy.push_back (0.01);
    loggy.push_back (0.05);
    loggy.push_back (0.1);
    loggy.push_back (0.5);
    loggy.push_back (1.0);
    loggy.push_back (5.0);
    loggy.push_back (10.0);
    loggy.push_back (50.0);
    loggy.push_back (0.0); // what if loggy contains a zero? scale falls over.

    morph::vvec<float> loggyout(loggy.size());
    try {
        ls.transform (loggy, loggyout);
        --rtn; // shouldn't get  here
        std::cout << "Unexpected: Log morph::scale given\n  " << loggy << ",\ntransforms it to\n  " << loggyout << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Caught expected error: " << e.what() << std::endl;
    }

    // Replace the offending 0 with a quiet NaN (NaNs are ok to be transformed; they come out still as NaNs)
    loggy.back() = std::numeric_limits<double>::quiet_NaN();
    ls.transform (loggy, loggyout);
    std::cout << "Log morph::scale given\n  " << loggy << ",\ntransforms it to\n  " << loggyout << std::endl;

    // That will have set the autoscale. now carry out inverse transform
    morph::vvec<float> range;
    range.push_back (0);
    range.push_back (0.2);
    range.push_back (0.4);
    range.push_back (0.6);
    range.push_back (0.8);
    range.push_back (1.0);

    morph::vvec<double> rangeout(range.size());
    ls.inverse (range, rangeout);

    std::cout << "Log morph::scale given\n  " << range << ",\n inverse transforms it to\n  " << rangeout << std::endl;

    auto li = range.begin();
    auto lio = rangeout.begin();
    // output in MATLAB/Octave format:
    std::cout << "[";
    while (li != range.end()) {
        std::cout << *li << "," << *lio << ";" << std::endl;
        ++li; ++lio;
    }
    std::cout << "];" << std::endl;;

    // Find scale that will transform -r -> +r to 0->1.
    morph::scale<double> d;
    double rmin = -3.0;
    double rmax = 5.0;
    d.compute_scaling (rmin, rmax);
    std::cout << "scale output for rmin: " << d.transform_one (rmin) << std::endl;
    std::cout << "scale output for rmin: " << d.transform_one (rmax) << std::endl;

    std::cout << "Inverse scale output for rmin: " << d.inverse_one (0) << std::endl;
    std::cout << "Inverse scale output for rmin: " << d.inverse_one (1) << std::endl;

    // Testing what happens to a NaN (after scaling should be nan)
    morph::scale<float> snan;
    snan.do_autoscale = true;
    std::vector<float> vfnan = {1,2,3,4,5,std::numeric_limits<float>::quiet_NaN(),9,18};
    std::vector<float> resultnan(vfnan);
    snan.transform (vfnan, resultnan);
    std::cout << "NaN containing data: Unscaled/scaled: ";
    for (unsigned int i = 0; i < vfnan.size(); ++i) {
        std::cout << vfnan[i]<<"/"<<resultnan[i]<<", ";
    }
    std::cout << std::endl;
    // Fifth element should be NaN still:
    if (!std::isnan(resultnan[5])) { --rtn; }

    morph::scale<int, float> sif;
    sif.output_range = morph::range<float>{0, 5};
    sif.compute_scaling (morph::range<int>{-10, 10});
    std::cout << "input 8(int) transforms to float: " << sif.transform_one (8) << std::endl;
    if (sif.transform_one (8) != 4.5f) { --rtn; }

    morph::scale<double, float> idsc1;
    idsc1.identity_scaling();

    std::cout << "(identity scaling) 2.0 transforms to " << idsc1.transform_one (2.0) << std::endl;
    std::cout << "(identity scaling) -2.0 transforms to " << idsc1.transform_one (-2.0) << std::endl;
    std::cout << "(identity scaling) -0.0 transforms to " << idsc1.transform_one (-0.0) << std::endl;

    std::cout << "(identity scaling) 2.1 inv transforms to " << idsc1.inverse_one (2.1f) << std::endl;
    std::cout << "(identity scaling) -2.2 inv transforms to " << idsc1.inverse_one (-2.2f) << std::endl;
    std::cout << "(identity scaling) -0.3 inv transforms to " << idsc1.inverse_one (-0.3f) << std::endl;

    morph::scale<float, double> idsc2;
    idsc2.identity_scaling();

    std::cout << "(identity scaling) 2.0 transforms to " << idsc2.transform_one (2.0f) << std::endl;
    std::cout << "(identity scaling) -2.0 transforms to " << idsc2.transform_one (-2.0f) << std::endl;
    std::cout << "(identity scaling) -0.0 transforms to " << idsc2.transform_one (-0.0f) << std::endl;

    std::cout << "(identity scaling) 2.1 inv transforms to " << idsc2.inverse_one (2.1) << std::endl;
    std::cout << "(identity scaling) -2.2 inv transforms to " << idsc2.inverse_one (-2.2) << std::endl;
    std::cout << "(identity scaling) -0.3 inv transforms to " << idsc2.inverse_one (-0.3) << std::endl;

    if (idsc1.transform_one (10.0) != 10.0f) { --rtn; }
    if (idsc1.inverse_one (3.475) != 3.475f) { --rtn; }
    std::cout << std::abs(idsc2.transform_one (-10.4f) - -10.4) << std::endl;
    std::cout << std::numeric_limits<float>::epsilon() << std::endl;
    // Have to allow a few epsilons here:
    if (std::abs(idsc2.transform_one (-10.4f) - -10.4) > 5 * std::numeric_limits<float>::epsilon()) { --rtn; }
    if (std::abs(idsc2.inverse_one (3.475f) - 3.475) > 5 * std::numeric_limits<float>::epsilon()) { --rtn; }

    morph::scale<morph::vec<float>> vecidsc;
    vecidsc.identity_scaling();
    std::cout << "(identity scaling) (1,1,1) transforms to " << vecidsc.transform_one (morph::vec<float>{1,1,1}) << std::endl;
    std::cout << "(identity scaling) (1,-1,1) transforms to " << vecidsc.transform_one (morph::vec<float>{1,-1,1}) << std::endl;
    if (vecidsc.transform_one (morph::vec<float>{1,-1,1}) != morph::vec<float>{1,-1,1}) { --rtn; }

    morph::scale<std::complex<float>> cpxidsc;
    cpxidsc.identity_scaling();
    std::cout << "(identity scaling) 1+2i transforms to " << cpxidsc.transform_one (std::complex<float>{1,2}) << std::endl;
    std::cout << "(identity scaling) 1-2i transforms to " << cpxidsc.transform_one (std::complex<float>{1,-2}) << std::endl;
    if (cpxidsc.transform_one (std::complex<float>{1,-2}) != std::complex<float>{1,-2}) { --rtn; }


    std::cout << "testScale " << (rtn == 0 ? "Passed" : "Failed") << std::endl;
    return rtn;
}
