// This is a graph which updates on each step. To test for a bug, but also to show how
// a graph can be completely redrawn each time, if required.
#include <morph/Visual.h>
#include <morph/GraphVisual.h>
#include <morph/vvec.h>
#include <morph/mathconst.h>

int main()
{
    morph::Visual v(1024, 768, "Continuous redrawing of GraphVisual");

    auto gv = std::make_unique<morph::GraphVisual<double>> (morph::vec<float>({0,0,0}));
    v.bindmodel (gv);

    morph::vvec<double> x;
    x.linspace (-morph::mathconst<double>::pi, morph::mathconst<double>::pi, 100);

    double dx = 0.0;

    morph::DatasetStyle ds_left;
    ds_left.datalabel = "sine left";
    gv->setdata (x, (x+dx).sin(), ds_left);

    morph::DatasetStyle ds_right;
    ds_right.axisside = morph::axisside::right;
    ds_right.linecolour = morph::colour::red2;
    ds_right.markercolour = morph::colour::red2;
    ds_right.datalabel = "sine right";
    gv->setdata (x, (x+dx).sin() -0.5, ds_right);

    // set style of the axis
    gv->axisstyle = morph::axisstyle::twinax;
    
    // Enable auto-rescaling of the x axis
    gv->auto_rescale_x = true;
    // Enable auto-rescaling of the y axis
    gv->auto_rescale_y = true;

    // rescale to fit data along the y axis
    gv->auto_rescale_fit = true;

    gv->finalize();

    auto gvp = v.addVisualModel (gv);

    while (v.readyToFinish == false) {
        dx += 0.01;
        v.waitevents (0.01667); // 16.67 ms ~ 60 Hz
        gvp->update (x+dx, (x+dx).sin() + dx, 0);
        gvp->update (x-dx, (x+dx).sin() - dx - 0.5, 1);
        v.render();
    }

    return 0;
}
