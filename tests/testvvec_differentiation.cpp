#include <morph/vvec.h>

int main()
{
    int rtn = 0;
    // Test differentiate.
    morph::vvec<float> a =     {1, 2, 3, 2, 1, 2, 3, 2, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 9, 7, 5, 1};
    morph::vvec<float> b = a;

    morph::vvec<float> diffexpct_wrap = {0.5,1,0,-1,0,1,0,-1,0,1,1,1,1,1,1,1,1,1,-0.5,-2,-2,-3,-2};
    morph::vvec<float> diffexpct_nowrap = {1,1,0,-1,0,1,0,-1,0,1,1,1,1,1,1,1,1,1,-0.5,-2,-2,-3,-4};

    b.diff_inplace(morph::vvec<float>::wrapdata::wrap);
    std::cout << "d/dx " << a << " = " << b << " (inplace, wrap)"<< std::endl;
    if (b != diffexpct_wrap) { rtn--; }
    morph::vvec<float> c = a.diff (morph::vvec<float>::wrapdata::wrap);
    std::cout << "d/dx " << a << " = " << c << " (rtnversion, wrap)"<< std::endl;
    if (c != diffexpct_wrap) { rtn--; }

    b = a;
    b.diff_inplace(morph::vvec<float>::wrapdata::none);
    std::cout << "d/dx " << a << " = " << b << " (inplace, NO wrap)"<< std::endl;
    if (b != diffexpct_nowrap) { rtn--; }
    c = a.diff (morph::vvec<float>::wrapdata::none);
    std::cout << "d/dx " << a << " = " << c << " (rtnversion, NO wrap)"<< std::endl;
    if (c != diffexpct_nowrap) { rtn--; }

    return rtn;
}
