#include <morph/vvec.h>

int main()
{
    int rtn = 0;

    morph::vvec<float> a = { 0.0f, 4.0f, -3.0f, 8.8f, -7.001f, -0.0f };

    morph::vvec<float> pp_e = { 0.0f, -3.0f, -7.001f, -0.0f };
    morph::vvec<float> pn_e = { 0.0f, 4.0f, 8.8f, -0.0f };

    morph::vvec<float> pp = a.prune_positive();
    morph::vvec<float> pn = a.prune_negative();

    std::cout << a << " pruned of positive elements: " << pp << std::endl;
    std::cout << a << " pruned of negative elements: " << pn << std::endl;

    if (pn != pn_e || pp != pp_e) { rtn--; }

    morph::vvec<float> bp = a;
    morph::vvec<float> bn = a;

    bn.prune_negative_inplace();
    bp.prune_positive_inplace();

    std::cout << a << " pruned of positive elements (in place): " << bp << std::endl;
    std::cout << a << " pruned of negative elements (in place): " << bn << std::endl;

    if (bn != pn_e || bp != pp_e) { rtn--; }

    return rtn;
}
