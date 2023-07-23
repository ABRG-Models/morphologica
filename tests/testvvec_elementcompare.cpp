#include <morph/vvec.h>
#include <iostream>

int main()
{
    int rtn = 0;

    morph::vvec<float> a = { 0.0f, 4.0f, -3.0f, 8.8f, -7.001f, -0.0f };

    morph::vvec<float> b = a.element_compare_gt (5.0f);

    morph::vvec<float> expct = { 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f };

    std::cout << a << " element-wise > 5.0f: " << b << std::endl;

    if (b != expct) { std::cerr << "Test fail!\n"; --rtn; }

    return rtn;
}
