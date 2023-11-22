/*
 * A dynamic, updating version of the scatter plot example
 */
#include <morph/Visual.h>
#include <morph/ColourMap.h>
#include <morph/ScatterVisual.h>
#include <morph/vec.h>
#include <morph/vvec.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <array>
#include <iostream>

int main()
{
    int rtn = -1;
    morph::Visual v(848, 480, "Moving ScatterVisual", {0,0}, {1,1,1}, 1.0f, 0.05f);
    v.zNear = 0.001;
    v.showCoordArrows = true;
    v.coordArrowsInScene = true;
    // Set a blueish background:
    v.bgcolour = {0.6f, 0.6f, 0.8f, 0.5f};
    v.lightingEffects();
    morph::vec<float, 3> offset = { 0.0, 0.0, 0.0 };

    // Do the initial set up of the ScatterVisual object
    auto sv = std::make_unique<morph::ScatterVisual<float>> (offset);
    v.bindmodel (sv);
    morph::vvec<morph::vec<float, 3>> points(20*20);
    morph::vvec<float> data(20*20);
    sv->setDataCoords (&points);
    sv->setScalarData (&data);
    sv->radiusFixed = 0.03f;
    sv->cm.setType (morph::ColourMapType::Plasma);
    // Finalize (build the model and add to the Visual), even though there's no data or points to show yet
    sv->finalize();
    auto svp = v.addVisualModel (sv); // When you add the model to the Visual, it takes
                                      // ownership of the memory and returns a pointer
                                      // that you can use (svp). svp is of type morph::ScatterVisual<float>*

    // Set a fixed scaling for the data value to colour conversion. This ensures that
    // the range of the data (which is about -0.42 to 0.42) maps to the range 0->1 which
    // is then passed into the morph::ColourMap. With the right scaling, we get the full
    // range of colours in the colour map. compute_autoscale() is the easiest way to do
    // this - you just pass in the min and max of the expected range. colourScale is an
    // object of type morph::Scale.
    svp->colourScale.compute_autoscale (-0.45f, 0.45f);

    unsigned int q = 0;
    while (!v.readyToFinish) {
        size_t k = 0;
        for (int i = -10; i < 10; ++i) {
            for (int j = -10; j < 10; ++j) {
                float x = 0.1f*i + 0.1f;
                float y = 0.1f*j;
                // z is some function of x, y
                float z = std::sin(q*morph::mathconst<float>::pi/100.0f) * x * std::exp(-(x*x) - (y*y));
                points[k] = {x, y, z};
                data[k] = z;
                k++;
            }
        }
        q++;

        // On each loop, just call VisualModel::reinit(). This re-builds the OpenGL
        // model using the now changed content of 'points' and 'data'
        svp->reinit();

        v.waitevents (0.016);
        v.render();
    }

    return rtn;
}
