// Example of 1D convolutions with vvec
#include <morph/Visual.h>
#include <morph/GraphVisual.h>
#include <morph/vvec.h>
#include <morph/mathconst.h>

int main()
{
    using mc = morph::mathconst<double>;
    using wrapdata = morph::vvec<double>::wrapdata;

    morph::vvec<double> x;
    x.linspace (-mc::pi, mc::pi-(mc::pi/5.0), 60);
    morph::vvec<double> y = x.sin();
    morph::vvec<double> r (x.size(), 0.0);
    r.randomize();
    y += r;

    morph::vvec<double> y2 = y.smooth_gauss (3.0, 3, wrapdata::wrap);

    // Graph x and y
    morph::Visual v(1024, 768, "Gaussian smoothing with morph::vvec");
    auto gv = new morph::GraphVisual<double> (v.shaderprog, v.tshaderprog, {0,0,0});
    gv->setdata (x, y, "raw");
    gv->setdata (x, y2, "smth");
    gv->finalize();
    v.addVisualModel (gv);
    v.render();
    v.keepOpen();

    return 0;
}
