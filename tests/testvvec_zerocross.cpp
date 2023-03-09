#include <morph/vvec.h>

int main()
{
    int rtn = 0;
    // Test zero crossings.    0  1  2  3  4   5   6   7   8  9 10 11
    morph::vvec<double> a =  {-1, 2, 3, 2, 0, -1, -2, -3, -2, 1, 2, 3};
    morph::vvec<float> expct_crossings = { +0.5, -4, +8.5};
    morph::vvec<float> crossings = a.zerocross ();
    std::cout << "crossings: " << crossings << " cf. expected: " << expct_crossings << std::endl;
    if (expct_crossings != crossings) { rtn--; }

    // Test zero crossings.    0  1  2  3  4   5   6   7   8  9 10 11
    morph::vvec<double> b =  {-1, 2, 3, 2, 0, -1, -2, -3, -2, 1, 2, 0};
    crossings = b.zerocross ();
    std::cout << "crossings: " << crossings << " cf. expected: " << expct_crossings << std::endl;
    if (expct_crossings != crossings) { rtn--; }

    // Test with wrappings
    morph::vvec<float> expct_crossings_wrap_a = { 0.5, -4, 8.5, -11.5};
    crossings = a.zerocross (morph::vvec<double>::wrapdata::wrap);
    std::cout << "for a = " << a << std::endl;
    std::cout << "(with wrap) crossings: " << crossings << " cf. expected: " << expct_crossings_wrap_a << std::endl;
    if (expct_crossings_wrap_a != crossings) { rtn--; }

    morph::vvec<float> expct_crossings_wrap_b = { 0.5, -4, 8.5, -11 };
    crossings = b.zerocross (morph::vvec<double>::wrapdata::wrap);
    std::cout << "crossings: " << crossings << " cf. expected: " << expct_crossings_wrap_b << std::endl;
    if (expct_crossings_wrap_b != crossings) { rtn--; }

    return rtn;
}
