#include "morph/MathAlgo.h"
#include <iostream>
#include <queue>
#include <list>
#include "morph/vec.h"

using namespace morph;
using namespace std;

// Testing MathAlgo with vector/array types.

int main()
{
    int rtn = 0;

    vector<double> vf;
    vf.push_back (0.0);
    vf.push_back (0.1);
    vf.push_back (0.2);
    vf.push_back (1.0);
    vf.push_back (1.1);
    vf.push_back (1.2);
    cout << "double functions" << endl;
    vec<double, 2> vfmm = MathAlgo::maxmin (vf);
    cout << "max/min: " << vfmm[0] << "/" << vfmm[1] << endl;

    vector<double> autoscaled = MathAlgo::autoscale (vf, 0.0, 1.0);
    vec<double, 2> vfmm2 = MathAlgo::maxmin (autoscaled);
    cout << "after autoscale, max/min: " << vfmm2[0] << "/" << vfmm2[1] << endl;
    if (vfmm2[0] != 1.0 || vfmm2[1] != 0.0) {
        rtn--;
    }

    // Throws runtime error, as expected; can't pass vector of scalars to the
    // MathAlgo::centroid function (instead, use MathAlgo::centroid3D)
    // OR better, removed the implementation and now we get a compile-time error.
    //double cencen = MathAlgo::centroid (vf);
    //cout << "centroid (" << cencen << endl;

    array<double, 3> v1 = { 1.0f, 1.0f, 1.0f };
    array<double, 3> v2 = { 0.5f, 2.0f, 1.0f };
    array<double, 3> v3 = { 1.0f, 1.0f, 2.1f };
    vector< array<double, 3> > vv3;
    vv3.push_back (v1);
    vv3.push_back (v2);
    vv3.push_back (v3);
    cout << "array<double,3> functions" << endl;
    vec<array<double,3>, 2 > vv3mm = MathAlgo::maxmin (vv3);
    cout << "max/min: (" << vv3mm[0][0] << ","
         << vv3mm[0][1] << "," << vv3mm[0][2] << ")/(" << vv3mm[1][0] << ","
         << vv3mm[1][1] << "," << vv3mm[1][2] << ")" << endl;
    if (abs(vv3mm[0][2] - 2.1) > 0.0000001
        || abs(vv3mm[1][2] - 1.0) > 0.0000001) {
        cout << "vv3mm[0][2] is " << vv3mm[0][2] << " not 2.1 OR" << endl;
        cout << "vv3mm[1][2] is " << vv3mm[1][2] << " not 1.0" << endl;
        --rtn;
    }

    array<double,3> cen = MathAlgo::centroid (vv3);
    cout << "centroid (" << cen[0] << "," << cen[1] << "," << cen[2] << ")" << endl;
    if (abs(cen[0] - 0.833333) > 0.0000007) {
        cout << "Error " << abs(cen[0] - 0.833333)<< endl;
        --rtn;
    }

    cout << "Try vector of vectors" << endl;
    vector<float> vf1 = { 1.0f, 1.0f, 1.0f };
    vector<float> vf2 = { 2.0f, 2.0f, 3.0f };
    vector<float> vf3 = { 3.0f, -1.0f, 5.0f };
    vector< vector<float> > vvf;
    vvf.push_back (vf1);
    vvf.push_back (vf2);
    vvf.push_back (vf3);
    cout << "vector<float> functions" << endl;
    vec<vector<float>, 2 > vvfmm = MathAlgo::maxmin (vvf);
    cout << "max: (";
    for (auto i : vvfmm[0]) {
        cout << i << " ";
    }
    cout << ")\n";
    cout << "min: (";
    for (auto i : vvfmm[1]) {
        cout << i << " ";
    }
    cout << ")\n";

    vector<float> cen2 = MathAlgo::centroid (vvf);
    cout << "centroid (" << cen2[0] << "," << cen2[1] << "," << cen2[2] << ")" << endl;

    vec<float> Vf1 = {0,0,0};
    vector<vec<float>> vVec3;
    Vf1.randomize();
    vVec3.push_back (Vf1);
    Vf1.randomize();
    vVec3.push_back (Vf1);
    Vf1.randomize();
    vVec3.push_back (Vf1);
    Vf1.randomize();
    vVec3.push_back (Vf1);
    vec<float> cen3 = MathAlgo::centroid (vVec3);
    cout << "Centroid of vector of vec<float> = " << cen3 << endl;

    list<int> li;
    li.push_back(2);
    li.push_back(1);
    li.push_back(7);
    vec<int,2> limm = MathAlgo::maxmin (li);
    cout << "max/min: " << limm[0] << "," << limm[1] << endl;

    deque<list<float>> qf;
    list<float> lv1 = {1,1}; // Hmm. list<float> has no [] operators.
    list<float> lv2 = {2,2};
    list<float> lv3 = {3,3};
    qf.push_back (lv1);
    qf.push_back (lv2);
    qf.push_back (lv3);

    list<float> lfcent = MathAlgo::centroid (qf);
    cout << "centroid: ";
    for (auto li : lfcent) {
        cout << li << " ";
    }
    cout << endl;

    deque<array<float, 2>> d2;
    d2.push_back ({1,1});
    d2.push_back ({2,2});
    d2.push_back ({3,3});
    deque<array<float, 2>> out = MathAlgo::autoscale (d2, 0.0f, 1.0f);

    cout << "autoscale on fixed size vectors:\n";
    for (auto d : out) {
        cout << "(" << d[0] << "," << d[1] << ")" << endl;
    }

    if (abs(out[1][1] - 0.353553) > 0.000005) {
        cout << "Error " << abs(out[1][1] - 0.353553)<< endl;
        --rtn;
    }

    vector<vector<float>> vv2;
    vv2.push_back ({1,1});
    vv2.push_back ({2,2});
    vv2.push_back ({3,3});
    vector<vector<float>> outvv = MathAlgo::autoscale (vv2, 0.0f, 1.0f);

    cout << "autoscale on dynamic vectors:\n";
    for (auto d : outvv) {
        cout << "(" << d[0] << "," << d[1] << ")" << endl;
    }


    return rtn;
}
