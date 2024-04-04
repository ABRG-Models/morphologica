// A showcase of different visual models

#include <morph/vvec.h>
#include <morph/mathconst.h>
#include <morph/loadpng.h>

#include <morph/Visual.h>

#include <morph/GraphVisual.h>

#include <morph/HexGrid.h>
#include <morph/HexGridVisual.h>

#include <morph/Grid.h>
#include <morph/GridVisual.h>

int main()
{
    using morph::unicode;

    morph::Visual v(1920, 1080, "morphologica showcase");

    /*
     * GraphVisual show-off
     */
    {
        auto gv = std::make_unique<morph::GraphVisual<double>> (morph::vec<float>({0,1,0}));
        v.bindmodel (gv);
        gv->axisstyle = morph::axisstyle::twinax;
        morph::vvec<double> x;
        x.linspace (-0.5, 0.8, 14);
        std::string ds1legend = unicode::toUtf8 (unicode::alpha) + "(x) = x" + unicode::toUtf8 (unicode::ss3);
        gv->setdata (x, x.pow(3), ds1legend);
        gv->ylabel = unicode::toUtf8 (unicode::alpha);
        std::string ds2legend = unicode::toUtf8 (unicode::beta) + "(x) = 100x" + unicode::toUtf8 (unicode::ss2);
        gv->setdata (x, x.pow(2)*100, ds2legend, morph::axisside::right);
        gv->ylabel2 = unicode::toUtf8 (unicode::beta);
        gv->addLabel ("morph::GraphVisual", morph::vec<float>({0,-0.25,0}), morph::TextFeatures(0.05));
        gv->finalize();
        v.addVisualModel (gv);
    }


    /*
     * HexGrid
     */
    {
        morph::HexGrid hg(0.01f, 3.0f, 0.0f);
        hg.setCircularBoundary (0.6f);
        // Make some dummy data (a sine wave) to make an interesting surface
        std::vector<float> data(hg.num(), 0.0f);
        for (unsigned int ri=0; ri<hg.num(); ++ri) {
            data[ri] = 0.05f + 0.05f*std::sin(20.0f*hg.d_x[ri]) * std::sin(10.0f*hg.d_y[ri]) ; // Range 0->1
        }
        auto hgv = std::make_unique<morph::HexGridVisual<float,morph::gl::version_4_1>>(&hg, morph::vec<float>({-2,-0.5,0}));
        v.bindmodel (hgv);
        hgv->setScalarData (&data);
        hgv->cm.setType (morph::ColourMapType::Inferno);
        hgv->hexVisMode = morph::HexVisMode::HexInterp; // Or morph::HexVisMode::Triangles for a smoother surface plot
        hgv->addLabel ("morph::HexGridVisual", morph::vec<float>({0,-0.7,0}), morph::TextFeatures(0.05));
        hgv->finalize();
        v.addVisualModel (hgv);
    }

    /*
     * Grid (columns perhaps)
     */
    {
        // Create a grid to show in the scene
        constexpr unsigned int Nside = 20;
        constexpr morph::vec<float, 2> grid_spacing = {0.05f, 0.05f};
        morph::Grid grid(Nside, Nside, grid_spacing);
        // Data
        std::vector<float> data(grid.n, 0.0);
        for (unsigned int ri=0; ri<grid.n; ++ri) {
            auto coord = grid[ri];
            float x = coord[0];
            float y = coord[1];
            data[ri] = 0.02f * std::exp (x) * std::exp (2*(y));
        }
        morph::vec<float, 3> offset = { -1.0f, -1.0f, 0.0f };
        auto gv = std::make_unique<morph::GridVisual<float>>(&grid, offset);
        v.bindmodel (gv);
        gv->gridVisMode = morph::GridVisMode::Columns;
        //gv->interpolate_colour_sides = true;
        gv->setScalarData (&data);
        gv->cm.setType (morph::ColourMapType::Twilight);
        gv->addLabel ("morph::GridVisual", morph::vec<float>({0,-0.1,0}), morph::TextFeatures(0.05));
        gv->finalize();
        v.addVisualModel (gv);
    }

    /*
     * GridVisual showing image
     */
    {
        morph::vec<float, 2> dx = { 0.005f, 0.005f };
        morph::vec<float, 2> nul = { 0.0f, 0.0f };
        morph::Grid g2(256U, 65U, dx, nul, morph::GridDomainWrap::Horizontal, // Triangles, BLTR
                       morph::GridOrder::bottomleft_to_topright);
        // Load an image
        std::string fn = "../examples/bike256_65.png";
        morph::vvec<float> image_data_bltr;
        morph::loadpng (fn, image_data_bltr, morph::vec<bool, 2>({false,true}));
        morph::vvec<float> image_data_neg = 1.0f - image_data_bltr;

        // Now visualise with a GridVisual
        auto gv2 = std::make_unique<morph::GridVisual<float>>(&g2, morph::vec<float>({0.2,-0.5,0}));
        v.bindmodel (gv2);
        gv2->gridVisMode = morph::GridVisMode::Pixels;
        gv2->setScalarData (&image_data_bltr);
        gv2->cm.setType (morph::ColourMapType::GreyscaleInv);
        gv2->zScale.setParams (0, 0);
        gv2->addLabel ("morph::GridVisual (flat, pixels)", morph::vec<float>({0,-0.1,0}), morph::TextFeatures(0.05));
        gv2->finalize();
        v.addVisualModel (gv2);
        auto gv3 = std::make_unique<morph::GridVisual<float>>(&g2, morph::vec<float>({0.2,-1,0}));
        v.bindmodel (gv3);
        gv3->gridVisMode = morph::GridVisMode::Columns;
        gv3->interpolate_colour_sides = true;
        gv3->setScalarData (&image_data_neg);
        gv3->cm.setType (morph::ColourMapType::GreyscaleInv);
        //gv3->zScale.setParams (0, 0);
        gv3->addLabel ("morph::GridVisual (negative, imaged as columns)", morph::vec<float>({0,-0.1,0}), morph::TextFeatures(0.05));
        gv3->finalize();
        v.addVisualModel (gv3);
    }

    /*
     * CurvyTellyVisual
     */

    /*
     * ScatterVisual (with axes)
     */

    /*
     * QuiverVisual (with axes)
     */

    /*
     * Render the scene on the screen until user quits with 'Ctrl-q'
     */
    v.keepOpen();

    return 0;
}
