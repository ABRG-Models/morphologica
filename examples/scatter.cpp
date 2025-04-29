/*
 * Visualize a test surface
 */
#include <morph/Visual.h>
#include <morph/ColourMap.h>
#include <morph/ScatterVisual.h>
#include <morph/scale.h>
#include <morph/vec.h>
#include <morph/vvec.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <array>

int main()
{
    int rtn = -1;

    morph::Visual v(1024, 768, "morph::ScatterVisual");
    v.zNear = 0.001;
    v.showCoordArrows (true);
    v.coordArrowsInScene (true);
    // Blueish background:
    v.bgcolour = {0.6f, 0.6f, 0.8f, 0.5f};
    v.lightingEffects();

    try {
        morph::vec<float, 3> offset = { 0.0, 0.0, 0.0 };
        morph::scale<float> scale1;
        scale1.setParams (1.0, 0.0);

        // Note use of morph::vvecs here, which can be passed into
        // VisualDataModel::setDataCoords(std::vector<vec<float>>* _coords)
        // and setScalarData(const std::vector<T>* _data)
        // This is possible because morph::vvec derives from std::vector.
        morph::vvec<morph::vec<float, 3>> points(20*20);
        morph::vvec<float> data(20*20);
        size_t k = 0;
        for (int i = -10; i < 10; ++i) {
            for (int j = -10; j < 10; ++j) {
                float x = 0.1*i;
                float y = 0.1*j;
                // z is some function of x, y
                float z = x * std::exp(-(x*x) - (y*y));
                points[k] = {x, y, z};
                data[k] = z;
                k++;
            }
        }

        auto sv = std::make_unique<morph::ScatterVisual<float>> (offset);
        v.bindmodel (sv);
        sv->setDataCoords (&points);
        sv->setScalarData (&data);

        sv->radiusFixed = 0.03f; // used in most markers as size

        // You can select which kind of object to show at the scatter locations. Default is a sphere
        sv->markers = morph::markerstyle::sphere;
        //sv->markers = morph::markerstyle::cube;
        //sv->markers = morph::markerstyle::tetrahedron;

        //sv->markers = morph::markerstyle::rod;
        // For a rod you may want to change the length with markerdirn:
        // sv->markerdirn *= 0.1f;
        // and reduce the radius:
        // sv->radiusFixed = 0.01f;

        sv->colourScale = scale1;
        sv->cm.setType (morph::ColourMapType::Plasma);
        sv->labelIndices = true;
        sv->finalize();
        v.addVisualModel (sv);

        v.keepOpen();

    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        rtn = -1;
    }

    return rtn;
}
