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

    std::cout << "rtn: " << rtn << std::endl;
    return rtn;
}
