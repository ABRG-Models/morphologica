#include <morph/vvec.h>
#include <morph/vec.h>
#include <array>
#include <vector>
#include <set>
#include <iostream>

int main()
{
    int rtn = 0;

    std::vector<float> svf = { 1, 2, 3, 4 };
    morph::vvec<float> mvf;
    mvf.set_from (svf);
    std::cout << "mvf set from std::vector: " << mvf << std::endl;
    if (mvf[0] != 1.0f || mvf[2] != 3.0f) {
        --rtn;
    }

    std::array<float, 10> af = { 2, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    mvf.set_from (af);
    std::cout << "mvf set from std::array: " << mvf << std::endl;
    if (mvf[0] != 2.0f || mvf[9] != 9.0f) {
        --rtn;
    }

    // set_from a scalar
    mvf.set_from (1.0f);
    std::cout << "mvf of size 10 set from float: " << mvf << std::endl;
    if (mvf[0] != 1.0f || mvf[2] != 1.0f) {
        --rtn;
    }

    // Create a set and fill
    std::set<float> sf;
    for (int i = 0; i < 12; ++i) {
        sf.insert (static_cast<float>(i));
    }

    mvf.set_from (sf);
    std::cout << "mvf of size 10 set from std::set: " << mvf << std::endl;
    if (mvf[0] != 0.0f || mvf[10] != 10.0f) {
        --rtn;
    }

    // set from a container of different type
    std::array<int, 10> ai = { 2, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    mvf.set_from (ai);
    std::cout << "mvf set from std::array<int, 10>: " << mvf << std::endl;
    if (mvf[0] != 2.0f || mvf[9] != 9.0f) {
        --rtn;
    }

    std::cout << "Test is returning: " << rtn << std::endl;
    return rtn;
}
