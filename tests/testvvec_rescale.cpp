#include <morph/vvec.h>

int main()
{
    int rtn = 0;

    morph::vvec<float> a = { 6,0,6 };

    morph::vvec<float> b = a;
    b.rescale();
    std::cout << a << " rescaled: " << b << std::endl;

    morph::vvec<float> c = a;
    c.rescale_sym();
    std::cout << a << " symetrically rescaled: " << c << std::endl;

    if (b != morph::vvec<float>({1,0,1})) { --rtn; }
    if (c != morph::vvec<float>({1,-1,1})) { --rtn; }

    return rtn;
}
