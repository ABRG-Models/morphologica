#include <vector>
using std::vector;
#include <list>
using std::list;
#include <array>
using std::array;
#include <iostream>
using std::cout;
using std::endl;
#include "morph/Scale.h"
using morph::Scale;

int main () {

    int rtn = 0;

    Scale<float> s;
    s.do_autoscale = true;
    vector<float> vf = {1,2,3.5,4,5.1,6.3,7};
    vector<float> result(vf);
    s.transform (vf, result);
    cout << "1st data: Unscaled/scaled: ";
    for (unsigned int i = 0; i < vf.size(); ++i) {
        cout << vf[i]<<"/"<<result[i]<<", ";
    }
    cout << endl;

    // Test scalar scaling
    if (result.back() != 1.0f) {
        cout << "Error in scalar scaling" << endl;
        rtn--;
    }

    // Different data, but extend max a bit. The result should now span >0,1
    // range. This shows that the autoscaling is carried out once only by the Scale
    // object. To autoscale again with vf2, set s.autoscaled=false
    vector<float> vf2 = {1,2.2,3.7,4.1,5.5,6.9,8};
    //s.autoscaled = false;   // would force re-autoscale when transform(vf2) next called
    //s.autoscale_from (vf2); // will immediately autoscale from vf2.
    s.transform (vf2, result);
    cout << "2nd data: Unscaled/scaled: ";
    for (unsigned int i = 0; i < vf2.size(); ++i) {
        cout << vf2[i]<<"/"<<result[i]<<", ";
    }
    cout << endl;

    Scale<array<float,4>> s2;
    s2.do_autoscale = true;
    vector<array<float,4>> vaf;
    vaf.push_back ({1,1,2,1});
    vaf.push_back ({2,2,2,3});
    vaf.push_back ({3,3,4,1});
    vaf.push_back ({4,4,4,4});
    vector<array<float,4>> result2(vaf);
    s2.transform (vaf, result2);

    cout << "vector<array<float,4>> unscaled/scaled vectors:\n";
    for (unsigned int i = 0; i < result2.size(); ++i) {

        cout << "(";
        for (auto v : vaf[i]) {
            cout << v << ",";
        }
        cout << ")   ";

        cout << "(";
        for (auto v : result2[i]) {
            cout << v << ",";
        }
        cout << ")\n";
    }

    // Test this scaling:
    vector<array<float, 4>>::const_iterator r2i = result2.end();
    r2i--; // To get to last element in vector
    float r2ilen = std::sqrt ((*r2i)[0] * (*r2i)[0] + (*r2i)[1] * (*r2i)[1] + (*r2i)[2] * (*r2i)[2] + (*r2i)[3] * (*r2i)[3]);
    if (abs(r2ilen - 1) > 0.0001) {
        cout << "Error" << endl;
        rtn--;
    }

    Scale<vector<double>> s3;
    s3.do_autoscale = true;
    list<vector<double>> vaf3;
    vaf3.push_back ({1,1,1});
    vaf3.push_back ({2,2,2});
    vaf3.push_back ({3,3,3});
    vaf3.push_back ({4,4,4});
    list<vector<double>> result3(vaf3);
    s3.transform (vaf3, result3);

    cout << "list<vector<double>> scaled vectors:\n";
    list<vector<double>>::iterator res3i = result3.begin();
    while (res3i != result3.end()) {
        cout << "(";
        vector<double>::iterator vi = res3i->begin();
        while (vi != res3i->end()) {
            cout << *vi++ << ",";
        }
        cout << ")\n";
        ++res3i;
    }

    // Log scaling
    Scale<double> ls;
    ls.do_autoscale = true;
    ls.setlog();

    list<double> loggy;
    loggy.push_back (0.01);
    loggy.push_back (1.0);

    list<double> loggyout(loggy);
    ls.transform (loggy, loggyout);

    // That will have set the autoscale. now carry out inverse transform
    vector<float> range;
    range.push_back (0);
    range.push_back (0.2);
    range.push_back (0.4);
    range.push_back (0.6);
    range.push_back (0.8);
    range.push_back (1.0);

    vector<float> rangeout(range);
    ls.inverse (range, rangeout);

    auto li = range.begin();
    auto lio = rangeout.begin();
    // output in MATLAB/Octave format:
    cout << "[";
    while (li != range.end()) {
        cout << *li << "," << *lio << ";" << endl;
        ++li; ++lio;
    }
    cout << "];" << endl;;

    return rtn;
}
