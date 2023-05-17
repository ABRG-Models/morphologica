#include "morph/MathAlgo.h"
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
    float sd = MathAlgo::compute_mean_sd<float> (vec, themean);
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

    MathAlgo::bubble_sort_lo_to_hi<float> (vec);

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

    MathAlgo::bubble_sort_hi_to_lo<float> (vec);

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
    MathAlgo::bubble_sort_lo_to_hi<float> (vec, indices);

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
    pair<float, float> mmvf = MathAlgo::maxmin (vf);
    cout << "vector has max: " << mmvf.first << " and min/lowest: " << mmvf.second << endl;
    if (mmvf.first == 0.9f && mmvf.second == -0.4f) {
        cout << "Max/min correct" << endl;
    } else {
        rtn--;
    }

    // A 2D box filter
    morph::vvec<float> vals = { 1, 2, 3, 2, 1,   4, 5, 6, 7, 4,   7, 4, 2, 1, 4,   8, 8, 6, 8, 3,   9, 8, 3, 2, 1  };
    morph::vvec<float> filtered (25, 0);
    morph::vvec<float> expect_result = { 17, 21, 25, 23, 19,  32, 34, 32, 30, 31,  47, 50, 47, 41, 46,  52, 55, 42, 30, 43,  37, 42, 35, 23, 31  };
    expect_result /= 9.0f;
    static constexpr int filter_width = 3;
    static constexpr int data_width = 5;
    morph::MathAlgo::boxfilter_2d<float, filter_width, data_width> (vals, filtered);
    if (filtered.sum() != expect_result.sum()) {
        std::cout << "filtered data: " << filtered << std::endl;
        std::cout << "expecting    : " << expect_result << std::endl;
        --rtn;
    } else {
        std::cout << "boxfilter is good\nfiltered data: " << filtered << std::endl;
        std::cout << "expecting    : " << expect_result << std::endl;
    }

    // 2D edge convolution test
    morph::vvec<float> edata = { 1.0f, 0.5f, 1.0f,
                                 0.5f, 1.0f, 0.4f,
                                 1.0f, 0.5f, 0.9f };
    morph::vvec<float> hedges (edata.size(), 0);
    morph::vvec<float> vedges (edata.size(), 0);

    morph::vvec<float> vedges_exp = { -1.0f+0.5f, -0.5f+1.0f, -1.0f+1.0f,
                                      -0.5f+1.0f, -1.0f+0.4f, -0.4f+0.5f,
                                      -1.0f+0.5f, -0.5f+0.9f, -0.9f+1.0f };
    morph::vvec<float> hedges_exp = { -1.0+0.5f, -0.5f+1.0f, -1.0f+0.4f,
                                      -0.5f+1.0f, -1.0f+0.5f, -0.4f+0.9f,
                                      0,          0,          0}; // no wrapping in vertical axis so top row of hedges is 0


    // Default template
    morph::MathAlgo::edgeconv_2d<float, 3> (edata, vedges, hedges);
    std::cout << "vert edges: " << vedges << std::endl;
    std::cout << "horz edges: " << hedges << std::endl;

    if (vedges != vedges_exp) { --rtn; }
    if (hedges != hedges_exp) { --rtn; }

    // Test version which inverts vertical edges
    constexpr bool invert_vert_edges = true;
    morph::MathAlgo::edgeconv_2d<float, 3, invert_vert_edges> (edata, vedges, hedges);
    if (vedges != -vedges_exp) { --rtn; }
    if (hedges != hedges_exp) { --rtn; }

    // Test version which inverts horizontal edges
    constexpr bool invert_horz_edges = true;
    morph::MathAlgo::edgeconv_2d<float, 3, false, invert_horz_edges> (edata, vedges, hedges);
    if (vedges != vedges_exp) { --rtn; }
    if (hedges != -hedges_exp) { --rtn; }

    // Test version which inverts vertical and horizontal edges
    morph::MathAlgo::edgeconv_2d<float, 3, invert_vert_edges, invert_horz_edges> (edata, vedges, hedges);
    if (vedges != -vedges_exp) { --rtn; }
    if (hedges != -hedges_exp) { --rtn; }

    return rtn;
}
