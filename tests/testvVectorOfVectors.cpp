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

    std::cout << vV << std::endl;
    std::cout << vV/2.0f << std::endl; // !!!!!

    return rtn;
}
