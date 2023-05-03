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

    morph::vvec<float> g = a;
    g.rescale_neg();
    std::cout << a << " neg rescaled: " << g << std::endl;

    if (b != morph::vvec<float>({1, 0, 1})) { --rtn; }
    if (c != morph::vvec<float>({1,-1, 1})) { --rtn; }
    if (g != morph::vvec<float>({0,-1, 0})) { --rtn; }

    morph::vvec<float> e = { -8,-7,-4,-2 };

    morph::vvec<float> d = e;
    d.rescale();
    std::cout << e << " rescaled: " << d << std::endl;

    morph::vvec<float> f = e;
    f.rescale_sym();
    std::cout << e << " sym rescaled: " << f << std::endl;

    morph::vvec<float> h = e;
    h.rescale_neg();
    std::cout << e << " neg rescaled: " << h << std::endl;

    return rtn;
}
