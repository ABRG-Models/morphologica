// Compute a Gaussian with a vvec
#include <morph/Visual.h>
#include <morph/GraphVisual.h>
#include <morph/vvec.h>
#include <morph/mathconst.h>

int main()
{
    morph::vvec<double> x;
    double sigma = 1.5;
    unsigned int nsigma = 3;
    double hw = sigma * nsigma;
    x.linspace (-hw, hw, 60);
    morph::vvec<double> y = x.gauss (sigma);

    // Graph x and y
    morph::Visual v(1024, 768, "1D convolutions with morph::vvec");
    auto gv = std::make_unique<morph::GraphVisual<double>> (v.shaderprog, v.tshaderprog, morph::vec<float>({0,0,0}));
    gv->setdata (x, y, "gauss");
    gv->finalize();
    v.addVisualModel (gv);
    v.keepOpen();

    return 0;
}
