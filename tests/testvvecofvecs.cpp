/*
 * Take a vvec of vecs. Divide the vvec by a scalar and get the vec members
 * of the vvec divided by a scalar.
 */

#include "morph/vvec.h"
#include "morph/vec.h"
using std::cout;
using std::endl;
using std::array;

int main()
{
    int rtn = 0;
    morph::vvec<morph::vec<float, 2>> vV;
    vV.push_back ({2.0f,4.1f});
    vV.push_back ({3.0f,6.1f});
    vV.push_back ({4.0f,8.1f});
    vV.push_back ({5.0f,12.1f});

    // Output the initial vector of vectors:
    std::cout << "Original vvec of vecs: " << vV << std::endl;
    // This divides each element of the vector of vectors vV by 2
    std::cout << "  (vvec of vecs) / 2 : " << vV/2.0f << std::endl;
    morph::vvec<morph::vec<float, 2>> vV2 = vV/2.0f;
    if (vV2[1][0] != 1.5f) { --rtn; }

    // Add/subtract vectors
    std::cout << "  (vvec of vecs) / + (1,-1) : " << vV + morph::vec<float, 2>({1,-1}) << std::endl;
    vV2 = vV + morph::vec<float, 2>({1,-1});
    if (vV2[1][0] != 4.0f) { --rtn; }

    std::cout << "  (vvec of vecs) / - (1,-1) : " << vV - morph::vec<float, 2>({1,-1}) << std::endl;
    vV2 = vV - morph::vec<float, 2>({1,-1});
    if (vV2[3][0] != 4.0f) { --rtn; }

    std::cout << "  (vvec of vecs) + 2.0f : " << vV + 2.0f << std::endl;
    vV2 = vV + 2.0f;
    if (vV2[2][1] != 10.1f) { --rtn; }

    std::cout << "  (vvec of vecs) - 10UL : " << vV - 10UL << std::endl;
    vV2 = vV - 10UL;
    if (vV2[2][0] != -6.0f) { --rtn; }

    // How about dividing a vvec of vecs by a vvec of scalars?
    morph::vvec<float> vf(vV.size());
    vf.linspace (0.0f, 3.0f);
    vV2 = vV * vf;
    std::cout << "  (vvec of vecs) * (vvec of scalars): " << vV2 << std::endl;
    if (vV2[2][0] != 8.0f) { --rtn; }

    vV2 = vV + vf;
    std::cout << "  (vvec of vecs) + (vvec of scalars): " << vV2 << std::endl;
    if (vV2[2][0] != 6.0f) { --rtn; }

    vV2 = vV - vf;
    std::cout << "  (vvec of vecs) - (vvec of scalars): " << vV2 << std::endl;
    if (vV2[2][0] != 2.0f) { --rtn; }

    // Division is ok like this:
    vV2 = vV * (float{1}/vf);
    std::cout << "  (vvec of vecs) * (1/(vvec of scalars)): " << vV2 << std::endl;
    if (vV2[2][0] != 2.0f) { --rtn; }

    // or like this:
    vV2 = vV / vf;
    std::cout << "  (vvec of vecs) / (vvec of scalars): " << vV2 << std::endl;
    if (vV2[2][0] != 2.0f) { --rtn; }

    // You can .zero() a vvec of vecs:
    morph::vvec<morph::vec<float, 2>> vV3 = { {1.0f, 1.0f}, {2.0f, 2.0f} };
    std::cout << "Before zero: " << vV3 << std::endl;
    vV3.zero();
    std::cout << "After zero: " << vV3 << std::endl;

    // Can you set_from(vec<>)? Previously no, but now, yes:
    vV3.set_from (morph::vec<float, 2>{5, 7});
    std::cout << "After set_from ({5,7}): " << vV3 << std::endl;
    if (vV3[0][0] != 5 || vV3[0][1] != 7 || vV3[1][0] != 5 || vV3[1][1] != 7) {
        --rtn;
    }

    // Test we can find max, min, longest, shortest of a vvec of vecs
    morph::vvec<morph::vec<double, 3>> vvshrt = { {-0,-0,6.78819124e-05}, {-0,1.78819124e-05,1.78819124e-05}, {0,6.78819124e-05,0}, {0,2,0}, {7.34092391e-05,0,0}, {6.78819124e-05,0,0}, {-6.78819124e-05,-0,0} };

    std::cout << "vvshrt max: " << vvshrt.max()   << " at index " << vvshrt.argmax() << std::endl;
    std::cout << "vvshrt longest: " << vvshrt.longest()  << " at index " << vvshrt.arglongest() << std::endl;
    std::cout << "vvshrt shortest: " << vvshrt.shortest() << " at index " << vvshrt.argshortest() << std::endl;
    std::cout << "vvshrt min: " << vvshrt.min() << " at index " << vvshrt.argmin() << std::endl;

    if (vvshrt.argmin() != 1) { --rtn; }
    if (vvshrt.argshortest() != 1) { --rtn; }
    if (vvshrt.argmax() != 3) { --rtn; }
    if (vvshrt.arglongest() != 3) { --rtn; }

    auto vrng = vvshrt.range();
    std::cout << "\nvvshrt range: " << vrng << std::endl;
    if (vrng.min == vvshrt[1] && vrng.max == vvshrt[3]) {
        // Good
    } else {
        --rtn;
    }

    morph::range<morph::vec<double, 3>> vextnts = vvshrt.extent();
    std::cout << "vextnts = " << vextnts << std::endl;

    if (vextnts.min == morph::vec<double, 3>{-6.78819124e-05, -0, 0}
        && vextnts.max == morph::vec<double, 3>{7.34092391e-05, 2, 6.78819124e-05}) {
        // Good
    } else {
        --rtn;
    }

    // Check scalar vvec::extent() function (which calls back to vvec::range())
    morph::range<float> vfr = morph::vvec<float>{1, 2, 3, 4}.extent();
    std::cout << "scalar range: " << vfr << std::endl;
    vfr = morph::vvec<float>{1, 2, -3, 4}.extent();
    std::cout << "scalar range: " << vfr << std::endl;

    // Test vector extent with an array of ints
    morph::vvec<std::array<int, 2>> vvai = {
        {-1, 1},
        {-3, 4},
        {-6, 2},
        {5,-4},
        {90, 8},
        {-7,-8}
    };
    std::array<int, 2> themin = {-7, -8};
    std::array<int, 2> themax = {90, 8};
    morph::range<std::array<int, 2>> vvair = vvai.extent();
    if (themin != vvair.min || themax != vvair.max) { --rtn; }

#if 0
    // Correctly does not compile because vvec<float> is not fixed size
    morph::vvec<morph::vvec<float>> vvvvf = {{-1,1},{-2,5,3}};
    auto vvvvfr = vvvvf.extent();
#endif

    // DOES compile - the vvec times vvec overload gets called and then called again, but then size
    // issues cause a runtime error which will alert the sleepy programmer that they were doing
    // something odd
    try {
        morph::vvec<morph::vvec<float>> vvvvf2 = {{-1,1},{-2,5,3}};
        morph::vvec<float>  vfac  = { 1, 2, 3 };
        auto result = vvvvf2 * vfac;
        --rtn;
    } catch (const std::exception& e) {
        std::cout << "Expected exception: " << e.what() << std::endl;
    }

    morph::vvec<morph::vec<int, 2>> vvfm = { {2, 3}, {4, 5} };
    morph::vec<int, 2> factor = {10, 100};
    morph::vvec<morph::vec<int, 2>> vvfm_result = (vvfm * factor);
    std::cout << vvfm << " * " << factor << " = " << vvfm_result << std::endl;
    if ((vvfm_result[0] == morph::vec<int, 2>{20, 300}
         && vvfm_result[1] == morph::vec<int, 2>{40, 500}) == false) {
        --rtn;
    }

    std::cout << "rtn: " << rtn << std::endl;
    return rtn;
}
