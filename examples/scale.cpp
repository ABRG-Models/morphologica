/*
 * Example usage of morph::Scale<>
 *
 * Author: Seb James
 * Date: September 2022
 */

#include <limits>
#include <vector>
#include <list>
#include <array>
#include <iostream>
#include <morph/Scale.h>

int main()
{
    int rtn = 0;

    // You can scale a set of numbers to the range 0->1: First create a Scale
    // object. This scales numbers of type float into scaled numbers also of type float.
    morph::Scale<float> s;
    // Set it to autoscale the next time its transform method is called
    s.do_autoscale = true;
    // Create a vector of numbers
    std::vector<float> vf = {1,2,3,4,5,8,9,18};
    // Create a vector for the result, of the same size as vf
    std::vector<float> result(vf);
    // Call s.transform to scale vf into result.
    s.transform (vf, result);
    for (unsigned int i = 0; i < vf.size(); ++i) {
        std::cout << vf[i] << " scales to: " << result[i] << "\n";
    }

    // Now create some different data, but make the maximum element bigger (32 instead
    // of 18). The result of s.transform() should now span a wider range than 0->1. This
    // shows that the autoscaling is carried out once only by the Scale object.
    std::vector<float> vf2 = {1,2,3,4,5,8,9,32};
    s.transform (vf2, result);

    for (unsigned int i = 0; i < vf2.size(); ++i) {
        std::cout << vf2[i] << " scales to: " << result[i] << "\n";
    }

    // If you need to reset the scaling in s (our Scale object), then you can do this:
    s.autoscale_from (vf2); // will immediately autoscale from the container of values vf2.

    // OR you can do this, which forces autoscale when s.transform() is next called.
    s.autoscaled = false;

    // Use this method to set the scaling if you know min and max of the range of
    // your input data:
    s.compute_autoscale (1.0f, 32.0f);

    // You can scale numbers between two different number types.
    morph::Scale<int,float> si;
    si.do_autoscale = true;
    std::vector<int> vfi = {-19,1,2,3,4,5,8,9,18};
    std::vector<float> resulti(vfi.size());
    si.transform (vfi, resulti);

    for (unsigned int i = 0; i < vfi.size(); ++i) {
        std::cout << "integer " << vfi[i] << " scales to floating point value " << resulti[i] << "\n";
    }

    //  You can scale arrays of numbers!
    morph::Scale<std::array<float,4>> s2;
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
        std::cout << ") scales to ";

        std::cout << "(";
        for (auto v : result2[i]) {
            std::cout << v << ",";
        }
        std::cout << ")\n";
    }


    // Log scaling
    morph::Scale<double, float> ls;
    ls.do_autoscale = true;
    ls.setlog();

    std::list<double> loggy;
    loggy.push_back (0.01);
    loggy.push_back (1.0);

    std::list<float> loggyout(loggy.size());
    ls.transform (loggy, loggyout);

    // That will have set the autoscale. now carry out inverse transform
    std::vector<float> range;
    range.push_back (0);
    range.push_back (0.2);
    range.push_back (0.4);
    range.push_back (0.6);
    range.push_back (0.8);
    range.push_back (1.0);

    std::vector<double> rangeout(range.size());
    ls.inverse (range, rangeout);

    auto li = range.begin();
    auto lio = rangeout.begin();
    // output in MATLAB/Octave format:
    std::cout << "[";
    while (li != range.end()) {
        std::cout << *li << "," << *lio << ";" << std::endl;
        ++li; ++lio;
    }
    std::cout << "];" << std::endl;

    return rtn;
}
