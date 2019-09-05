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
    float sd = MathAlgo<float>::compute_sd (vec);
    cout << "Standard deviation: " << sd << endl << endl;
    if (abs(sd - 0.454863) > 0.000000406) {
        cout << "Wrong SD" << endl;
        rtn--;
    }
    //cout << "SD error: " << (sd-0.454863) << endl;

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

    return rtn;
}
