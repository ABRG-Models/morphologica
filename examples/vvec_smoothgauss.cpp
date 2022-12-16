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

    wrapdata w = wrapdata::wrap;
    morph::vvec<double> yd = y2;
    yd.diff_inplace (w);
    yd*=1;

    morph::vvec<double> yd2 = y2.diff (w);
    yd2 += 0.5;


    // Graph x and y
    morph::Visual v(1024, 768, "Gaussian smoothing with morph::vvec");
    auto gv = std::make_unique<morph::GraphVisual<double>> (v.shaderprog, v.tshaderprog, morph::vec<float>({0,0,0}));
    gv->setdata (x, y, "raw");
    gv->setdata (x, y2, "smth");
    gv->setdata (x, yd, "smthdiff inplace");
    gv->setdata (x, yd2, "smthdiff");
    gv->finalize();
    v.addVisualModel (gv);
    v.render();
    v.keepOpen();

    return 0;
}
