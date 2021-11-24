/*
 * Take a vVector of Vectors. Divide the vVector by a scalar and get the Vector members
 * of the vVector divided by a scalar.
 */

#include "morph/vVector.h"
#include "morph/Vector.h"
using std::cout;
using std::endl;
using std::array;

int main()
{
    int rtn = 0;
    morph::vVector<morph::Vector<float, 2>> vV;
    vV.push_back ({2.0f,4.1f});
    vV.push_back ({3.0f,6.1f});
    vV.push_back ({4.0f,8.1f});
    vV.push_back ({5.0f,12.1f});

    // Output the initial vector of vectors:
    std::cout << "Original vVector of Vectors: " << vV << std::endl;
    // This divides each element of the vector of vectors vV by 2
    std::cout << "  (vVector of Vectors) / 2 : " << vV/2.0f << std::endl;
    morph::vVector<morph::Vector<float, 2>> vV2 = vV/2.0f;
    if (vV2[1][0] != 1.5f) { --rtn; }

    // Add/subtract vectors
    std::cout << "  (vVector of Vectors) / + (1,-1) : " << vV + morph::Vector<float, 2>({1,-1}) << std::endl;
    vV2 = vV + morph::Vector<float, 2>({1,-1});
    if (vV2[1][0] != 4.0f) { --rtn; }

    std::cout << "  (vVector of Vectors) / - (1,-1) : " << vV - morph::Vector<float, 2>({1,-1}) << std::endl;
    vV2 = vV - morph::Vector<float, 2>({1,-1});
    if (vV2[3][0] != 4.0f) { --rtn; }

    std::cout << "  (vVector of Vectors) + 2.0f : " << vV + 2.0f << std::endl;
    vV2 = vV + 2.0f;
    if (vV2[2][1] != 10.1f) { --rtn; }

    std::cout << "  (vVector of Vectors) - 10UL : " << vV - 10UL << std::endl;
    vV2 = vV - 10UL;
    if (vV2[2][0] != -6.0f) { --rtn; }

    // How about dividing a vVector of Vectors by a vVector of scalars?
    morph::vVector<float> vf(vV.size());
    vf.linspace (0.0f, 3.0f);
    vV2 = vV * vf;
    std::cout << "  (vVector of Vectors) * (vVector of scalars): " << vV2 << std::endl;
    if (vV2[2][0] != 8.0f) { --rtn; }

    vV2 = vV + vf;
    std::cout << "  (vVector of Vectors) + (vVector of scalars): " << vV2 << std::endl;
    if (vV2[2][0] != 6.0f) { --rtn; }

    vV2 = vV - vf;
    std::cout << "  (vVector of Vectors) - (vVector of scalars): " << vV2 << std::endl;
    if (vV2[2][0] != 2.0f) { --rtn; }

    // Division is ok like this:
    vV2 = vV * (float{1}/vf);
    std::cout << "  (vVector of Vectors) * (1/(vVector of scalars)): " << vV2 << std::endl;
    if (vV2[2][0] != 2.0f) { --rtn; }

    // or like this:
    vV2 = vV / vf;
    std::cout << "  (vVector of Vectors) / (vVector of scalars): " << vV2 << std::endl;
    if (vV2[2][0] != 2.0f) { --rtn; }

    std::cout << "rtn: " << rtn << std::endl;
    return rtn;
}
