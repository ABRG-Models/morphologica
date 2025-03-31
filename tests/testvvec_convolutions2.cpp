#include <morph/vvec.h>

int main()
{
    int rtn = 0;
    // Test convolve with non-odd kernel which is now size 4 elements
    morph::vvec<float> a = { 1, 2, 3 };
    morph::vvec<float> b = { 2, 3, 2, 4 };
    morph::vvec<float> r1expct = { 14, 17, 14 };
    morph::vvec<float> r1 = a.convolve (b); // wrapdata::none, centre_kernel::yes, resize_output::no
    if (r1 != r1expct) { rtn -= 1; }
    std::cout << a << " * " << b << " = " << r1 << " (no wrap, centre, no resize, expect " << r1expct << ")" << std::endl;

    morph::vvec<float> r1_pure_expct = { 2, 7, 14, 17, 14, 12 };
    morph::vvec<float> r1p = a.convolve<morph::vvec<float>::wrapdata::none,
                                        morph::vvec<float>::centre_kernel::no,
                                        morph::vvec<float>::resize_output::yes> (b);
    if (r1p != r1_pure_expct) { rtn -= 1; }
    std::cout << a << " * " << b << " = " << r1p << " (no wrap, no centre, resize, expect " << r1_pure_expct << ")" << std::endl;

    morph::vvec<float> aa = { 1, 2, 3, 4, 5 }; // Can't do 1,2,3 as this is now smaller than 4 element kernel (and we wrap)
    morph::vvec<float> r2expct = { 37, 33, 34, 25, 36 };
    morph::vvec<float> r2 = aa.convolve<morph::vvec<float>::wrapdata::wrap,
                                        morph::vvec<float>::centre_kernel::no,
                                        morph::vvec<float>::resize_output::no> (b);
    std::cout << aa << " * " << b << " = " << r2  << " (wrap, no centre, no resize, expect " << r2expct << ")" << std::endl;
    if (r2 != r2expct) {  std::cout << "r2 " << r2 << " != " << r2expct << std::endl; rtn -= 1; }

    r2expct = { 34, 25, 36, 37, 33 };
    r2 = aa.convolve<morph::vvec<float>::wrapdata::wrap, // wrapping complexified if kernel > data
                     morph::vvec<float>::centre_kernel::yes,
                     morph::vvec<float>::resize_output::no> (b);
    std::cout << a << " * " << b << " = " << r2  << " (wrap, centre, expect " << r2expct << ")" << std::endl;
    if (r2 != r2expct) {  std::cout << "r2 " << r2 << " != " << r2expct << std::endl; rtn -= 1; }

    // convolve in place equivalent:
    r2 = aa;
    r2.convolve_inplace<morph::vvec<float>::wrapdata::wrap,
                        morph::vvec<float>::centre_kernel::yes,
                        morph::vvec<float>::resize_output::no> (b);
    if (r2 != r2expct) { std::cout << "r2 inplace " << r2 << " != " << r2expct << std::endl; rtn -= 1; }

    // Test convolve in place
    r1 = a;
    r1.convolve_inplace (b);
    if (r1 != r1expct) { std::cout << "r1 inplace " << r1 << " != " << r1expct << std::endl; rtn -= 1; }

    // Pure maths convolution
    morph::vvec<float> a2 = {4, 5, 6}; // 3blue1brown example kernel!
    morph::vvec<float> threeb1b = a.convolve<morph::vvec<float>::wrapdata::none,
                                             morph::vvec<float>::centre_kernel::no,
                                             morph::vvec<float>::resize_output::yes> (a2);
    morph::vvec<float> threeb1b_expct = { 4, 13, 28, 27, 18 };
    std::cout <<  "pure convolution " << a << " * " << a2 << " = " << threeb1b << " (expect " << threeb1b_expct << ")\n";

    std::cout << (rtn ? "FAIL\n" : "PASS\n");
    return rtn;
}
