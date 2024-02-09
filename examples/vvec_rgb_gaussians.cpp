// Compute a Gaussian with a vvec
#include <morph/Visual.h>
#include <morph/GraphVisual.h>
#include <morph/vvec.h>
#include <morph/mathconst.h>

int main()
{
    morph::vvec<float> rgauss (360, 0.0f);
    float sigma = 45.0f;
    rgauss.linspace (-180.0f, 179.0f, 360);
    rgauss.gauss_inplace (sigma);
    rgauss.rotate (180);
    morph::vvec<float> ggauss = rgauss;
    ggauss.rotate (-120);
    morph::vvec<float> bgauss = ggauss;
    bgauss.rotate (-120);

    // For x axis
    morph::vvec<float> x;
    x.linspace (0.0f, 359.0f, 360);

    // Graph x and y
    morph::Visual v(1024, 768, "RGB");
    auto gv = std::make_unique<morph::GraphVisual<float>> (morph::vec<float>({0,0,0}));
    v.bindmodel (gv);
    morph::DatasetStyle ds(morph::stylepolicy::markers);
    ds.datalabel = "R";
    ds.markercolour = morph::colour::crimson;
    gv->setdata (x, rgauss, ds);

    ds.datalabel = "G";
    ds.markercolour = morph::colour::springgreen;
    gv->setdata (x, ggauss, ds);

    ds.datalabel = "B";
    ds.markercolour = morph::colour::royalblue;
    gv->setdata (x, bgauss, ds);

    gv->finalize();
    v.addVisualModel (gv);
    v.keepOpen();

    return 0;
}
