#include <morph/vvec.h>

int main()
{
    int rtn = 0;
    // Test convolve
    morph::vvec<float> a = {1, 2, 3};
    morph::vvec<float> b = {2, 3, 2};
    morph::vvec<float> r1expct = {7, 14, 13};
    morph::vvec<float> r1 = a.convolve (b);
    if (r1 != r1expct) { rtn -= 1; }
    std::cout << a << " * " << b << " = " << r1 << " (no wrap, expect " << r1expct << ")" << std::endl;

    morph::vvec<float> r1_pure_expct = {2, 7, 14, 13, 6};
    morph::vvec<float> r1p = a.convolve<morph::vvec<float>::wrapdata::none, true> (b);
    if (r1p != r1_pure_expct) { rtn -= 1; }
    std::cout << a << " * " << b << " = " << r1p << " (no wrap, pure expect " << r1_pure_expct << ")" << std::endl;

    morph::vvec<float> r2expct = {13, 14, 15};
    morph::vvec<float> r2 = a.convolve<morph::vvec<float>::wrapdata::wrap> (b);
    std::cout << a << " * " << b << " = " << r2  << " (wrap, expect " << r2expct << ")" << std::endl;
    if (r2 != r2expct) { rtn -= 1; }

    // Test convolve in place
    r1 = a;
    r2 = a;
    r1.convolve_inplace (b);
    r2.convolve_inplace<morph::vvec<float>::wrapdata::wrap> (b);
    if (r1 != r1expct) { rtn -= 1; }
    if (r2 != r2expct) { rtn -= 1; }

    // Pure maths convolution
    morph::vvec<float> a2 = {4, 5, 6}; // 3blue1brown example kernel!
    morph::vvec<float> threeb1b = a.convolve<morph::vvec<float>::wrapdata::none, true> (a2);
    morph::vvec<float> threeb1b_expct = { 4, 13, 28, 27, 18 };
    std::cout <<  "pure convolution " << a << " * " << a2 << " = " << threeb1b << " (expect " << threeb1b_expct << ")\n";

    std::cout << (rtn ? "FAIL\n" : "PASS\n");
    return rtn;
}
