#include "MathAlgo.h"
#include <iostream>

using namespace morph;
using namespace std;

int main()
{
    int rtn = 0;

    float first  = 0.4;
    float second = 0.3;
    float third  = 0.89;
    float fourth = 0.63;

    vector<float> vec = {{ first, second, third, fourth }};

    // SD of the vector:
    float themean = 0.0f;
    float sd = MathAlgo<float>::compute_mean_sd (vec, themean);
    cout << "Standard deviation: " << sd << " and mean: " << themean << endl;
    if (abs(sd - 0.262615053) > numeric_limits<float>::epsilon()) {
        cout << "Wrong SD" << endl;
        rtn--;
    }
    //cout << "SD differs from expected value by: " << (sd-0.262615053)
    //     << " (float's epsilon is " << numeric_limits<float>::epsilon() << ")" << endl;

    cout << "Before sort";
    for (auto v : vec) {
        cout << ", " << v;
    }
    cout << endl;

    MathAlgo<float>::bubble_sort_lo_to_hi (vec);

    cout << "After sort lo to hi";
    for (auto v : vec) {
        cout << ", " << v;
    }
    cout << endl;

    if (vec[0] == second && vec[1] == first && vec[2] == fourth && vec[3] == third) {
        cout << "Order correct" << endl;
    } else {
        rtn--;
    }

    MathAlgo<float>::bubble_sort_hi_to_lo (vec);

    cout << "After sort hi to lo";
    for (auto v : vec) {
        cout << ", " << v;
    }
    cout << endl;

    if (vec[0] == third && vec[1] == fourth && vec[2] == first && vec[3] == second) {
        cout << "Order correct" << endl;
    } else {
        rtn--;
    }

    // Reset vec
    vec = {{ first, second, third, fourth }};
    vector<unsigned int> indices(vec.size(), 0);
    MathAlgo<float>::bubble_sort_lo_to_hi (vec, indices);

    cout << "After sort lo to hi of INDICES:" << endl;
    for (unsigned int i = 0; i < vec.size(); ++i) {
        cout << "val " << vec[i] << " has index " << indices[i] << endl;
    }

    cout << "Should be in order:" << endl;
    for (unsigned int i = 0; i < vec.size(); ++i) {
        cout << i << ": " << vec[indices[i]] << endl;
    }

    if (vec[indices[0]] == second && vec[indices[1]] == first && vec[indices[2]] == fourth && vec[indices[3]] == third) {
        cout << "Order correct" << endl;
    } else {
        rtn--;
    }

    vector<float> vf = {0.1f, 0.2f, 0.9f, -0.4f};
    pair<float, float> mmvf = MathAlgo<float>::maxmin (vf);
    cout << "vector has max: " << mmvf.first << " and min/lowest: " << mmvf.second << endl;
    if (mmvf.first == 0.9f && mmvf.second == -0.4f) {
        cout << "Max/min correct" << endl;
    } else {
        rtn--;
    }

    return rtn;
}
