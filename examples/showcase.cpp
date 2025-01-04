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

#include <morph/TriaxesVisual.h>
#include <morph/ScatterVisual.h>

// A simple Izhikevich neuron model class used below
struct izhi
{
    // state variables
    float I = 0.0f;      // Input current
    float u = -10.0f;    // 'The refractory variable'
    float v = -70.0f;    // 'Membrane voltage'

    // Parameters. In 'abc' model statement format.
    static constexpr float a = 0.03f;
    static constexpr float b = 0.193f;
    static constexpr float c = -65.0f;
    static constexpr float d = 0.05f;

    static constexpr float A = 0.032f;
    static constexpr float B = 4.0f;
    static constexpr float C = 113.147f;

    static constexpr float T = 0.4f;
    static constexpr float SI = 5.0f;
    static constexpr float vpeak = 30.0f;

    // Derived parameters
    static constexpr float AT = A * T;
    static constexpr float BT = B * T;
    static constexpr float CT = C * T;
    static constexpr float ToverSI = T/SI;

    // vdot and udot computations
    float dv (const float _u, const float _v) { return AT * _v * _v + BT * _v + CT - _u * T + I * ToverSI; }
    float du (const float _u, const float _v) { return a * T * (b * _v - _u); }

    // Apply one timestep of the differential equations for the model
    void step()
    {
        bool spike = (v > vpeak);       // This is the reset condition
        float _du = this->du (u, v);    // Compute now as in next line, v may change
        v = spike ? c       : (v + this->dv (u, v));
        u = spike ? (u + d) : (u + _du);
    }

    // Compute nullclines. For Vn, the given input membrane voltages, return u and v nullclines in u_nc and v_nc
    void nullclines (const morph::vvec<float>& Vn, morph::vvec<float>& u_nc,  morph::vvec<float>& v_nc)
    {
        u_nc.resize (Vn.size(), 0.0f);
        v_nc.resize (Vn.size(), 0.0f);
        for (unsigned int i = 0; i < Vn.size(); ++i) {
            v_nc[i] = A * Vn[i] * Vn[i] + B * Vn[i] + C + I / SI;
            u_nc[i] = Vn[i] * b;
        }
    }

    // Compute the vectorfield of du and dv vs. u and v
    void vectorfield (const morph::vvec<float>& _u, const morph::vvec<float>& _v,
                      morph::vvec<morph::vec<float, 2>>& vecfield)
    {
        if (_u.size() != _v.size()) { return; }
        vecfield.resize (_u.size() * _v.size(), {0,0});
        for (unsigned int j = 0; j < _u.size(); ++j) {
            unsigned int shft = j * _v.size();
            for (unsigned int i = 0; i < _v.size(); ++i) {
                vecfield[i + shft] = { this->dv (_u[j], _v[i]), this->du (_u[j], _v[i]) };
            }
        }
    }
}; // class izhi

int main()
{
    using morph::unicode;

    morph::Visual v(1920, 1080, "morphologica showcase");
    v.setSceneTrans (morph::vec<float,3>({1.30124f, -0.730136f, -8.2f}));
    v.lightingEffects();

    /*
     * GraphVisual show-off
     */
    {
        auto gv1 = std::make_unique<morph::GraphVisual<double>> (morph::vec<float>({0,1,0}));
        v.bindmodel (gv1);
        gv1->axisstyle = morph::axisstyle::twinax;
        gv1->setsize (1.6, 1.6);
        morph::vvec<double> x;
        x.linspace (-0.5, 0.8, 14);
        std::string ds1legend = unicode::toUtf8 (unicode::alpha) + "(x) = x" + unicode::toUtf8 (unicode::ss3);
        gv1->setdata (x, x.pow(3), ds1legend);
        gv1->ylabel = unicode::toUtf8 (unicode::alpha);
        std::string ds2legend = unicode::toUtf8 (unicode::beta) + "(x) = 100x" + unicode::toUtf8 (unicode::ss2);
        gv1->setdata (x, x.pow(2)*100, ds2legend, morph::axisside::right);
        gv1->ylabel2 = unicode::toUtf8 (unicode::beta);
        gv1->addLabel ("morph::GraphVisual with morph::axisstyle::twinax", morph::vec<float>({0,-0.25,0}), morph::TextFeatures(0.05));
        gv1->finalize();
        v.addVisualModel (gv1);
    }

    /*
     * HexGrid
     */
    {
        morph::HexGrid hg(0.06f, 3.0f, 0.0f);
        hg.setCircularBoundary (0.6f);
        // Make some dummy data (a sine wave) to make an interesting surface
        std::vector<float> data(hg.num(), 0.0f);
        for (unsigned int ri=0; ri<hg.num(); ++ri) {
            data[ri] = 0.05f + 0.15f*std::sin(10.0f*hg.d_x[ri]) * std::sin(1.8f*hg.d_y[ri]) ; // Range 0->1
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
     * Grid, with column view
     */
    {
        // Create a grid to show in the scene
        constexpr unsigned int Nside = 20;
        constexpr morph::vec<float, 2> grid_spacing = {0.05f, 0.05f};
        morph::Grid grid(Nside, Nside, grid_spacing);
        // Data
        std::vector<float> data(grid.n(), 0.0);
        for (unsigned int ri=0; ri<grid.n(); ++ri) {
            auto coord = grid[ri];
            float x = coord[0];
            float y = coord[1];
            data[ri] = 0.02f * std::exp (x) * std::exp (2*(y));
        }
        morph::vec<float, 3> offset = { -1.1f, -1.0f, 0.0f };
        auto gv = std::make_unique<morph::GridVisual<float>>(&grid, offset);
        v.bindmodel (gv);
        gv->gridVisMode = morph::GridVisMode::Columns;
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
        morph::vvec<float> image_data;
        morph::loadpng (fn, image_data, morph::vec<bool, 2>({false,true}));

        // Now visualise with a GridVisual
        auto gv2 = std::make_unique<morph::GridVisual<float>>(&g2, morph::vec<float>({0.2,-0.5,0}));
        v.bindmodel (gv2);
        gv2->gridVisMode = morph::GridVisMode::Pixels;
        gv2->setScalarData (&image_data);
        gv2->cm.setType (morph::ColourMapType::GreyscaleInv);
        gv2->zScale.setParams (0, 0);
        gv2->addLabel ("morph::GridVisual (flat, pixels)", morph::vec<float>({0,-0.1,0}), morph::TextFeatures(0.05));
        gv2->finalize();
        v.addVisualModel (gv2);
        auto gv3 = std::make_unique<morph::GridVisual<float>>(&g2, morph::vec<float>({0.2,-1,0}));
        v.bindmodel (gv3);
        gv3->gridVisMode = morph::GridVisMode::Columns;
        gv3->interpolate_colour_sides (true);
        gv3->setScalarData (&image_data);
        gv3->cm.setType (morph::ColourMapType::Plasma);
        gv3->zScale.setParams (0.1, 0); // Reduce height in 'z'
        gv3->addLabel ("morph::GridVisual (columns)", morph::vec<float>({0,-0.1,0}), morph::TextFeatures(0.05));
        gv3->finalize();
        v.addVisualModel (gv3);
    }

    /*
     * CurvyTellyVisual
     */

    /*
     * ScatterVisual (with axes)
     */
    // First the Triaxes:
    auto scat_offs = morph::vec<float>({-4,-1.0,0});
    auto tav = std::make_unique<morph::TriaxesVisual<float>>(scat_offs);
    v.bindmodel (tav);
    tav->axisstyle = morph::axisstyle::L;
    // Specify axes min and max with a min and max vector
    //                                         x      y       z
    tav->input_min = morph::vec<float, 3>({ -1.0f,  0.0f,   0.0f });
    tav->input_max = morph::vec<float, 3>({  1.0f, 10.0f, 100.0f });
    // Set the axis labels
    tav->xlabel = "x";
    tav->ylabel = "y";
    tav->zlabel = "z";
    tav->finalize();
    v.addVisualModel (tav);
    // Second the scatter vis:
    auto sv = std::make_unique<morph::ScatterVisual<float>> (scat_offs);
    v.bindmodel (sv);
    morph::vvec<morph::vec<float, 3>> points(20*20);
    morph::vvec<float> data(20*20);
    sv->setDataCoords (&points);
    sv->setScalarData (&data);
    sv->radiusFixed = 0.03f;
    sv->cm.setType (morph::ColourMapType::Plasma);
    sv->finalize(); // no data yet
    auto svp = v.addVisualModel (sv); // We will use the pointer, svp to update the graph

    /*
     * GraphVisual including plotting quivers
     */
    constexpr unsigned int N = 1000;
    constexpr bool twodee = true;

    // Perform an Izhikevich neuron model simulation
    morph::vvec<float> _u(N, 0.0f);
    morph::vvec<float> _v(N, 0.0f);
    izhi iz;
    for (unsigned int i = 0; i < N; ++i) {
        iz.step();
        _v[i] = iz.v;
        _u[i] = iz.u;
    }
    // Compute nullclines
    morph::vvec<float> u_nc;
    morph::vvec<float> v_nc;
    morph::vvec<float> vrng;
    vrng.linspace (-80.0f, -20.0f, 1000);
    iz.nullclines (vrng, u_nc, v_nc);
    // Compute du/dv vector field
    static constexpr size_t qN = 50;
    constexpr float umin = -15.6f;
    constexpr float umax = -3.6f;
    constexpr float vmin = -80.0f;
    constexpr float vmax = -20.0f;
    morph::vvec<float> qurng; // y axis
    morph::vvec<float> qvrng; // x axis
    qvrng.linspace (vmin, vmax, qN);
    qurng.linspace (umin, umax, qN);
    morph::vvec<morph::vec<float, 2>> du_dv_vecfield;
    iz.vectorfield (qurng, qvrng, du_dv_vecfield);
    morph::vec<float, 2> gridspacing = {
        (vmax - vmin) / (qN-1),
        (umax - umin) / (qN-1)
    };
    morph::vec<float, 2> gridzero = { vmin, umin };
    morph::Grid<unsigned int, float> grid (qN, qN, gridspacing, gridzero);

    // Visualize results
    morph::vvec<float> t(N, 0.0f);
    t.linspace (0.0f, N/100.0f, N);

    // Set default dataset graphing styles
    morph::DatasetStyle ds;
    ds.linewidth = 0.003f;
    ds.linecolour = morph::colour::grey30;
    ds.markersize = 0.015f;
    ds.markerstyle = morph::markerstyle::uphexagon;

    // Graph membrane voltage vs. time
    morph::vec<float> izoff = {-4, 1, 0};
    auto gv = std::make_unique<morph::GraphVisual<float>> (morph::vec<float>({0,0,0})+izoff);
    v.bindmodel (gv);
    gv->twodimensional = twodee;
    gv->setsize (1,0.8);
    gv->xlabel = "t";
    gv->ylabel = "v";
    ds.datalabel = "v(t)";
    ds.markerstyle = morph::markerstyle::diamond;
    gv->setdata (t, _v, ds);
    gv->finalize();
    gv->addLabel ("using morph::stylepolicy::both\nand morph::markerstyle::diamond", morph::vec<float>({0,-0.25,0}), morph::TextFeatures(0.05));
    v.addVisualModel (gv);

    // Graph u(t)
    auto gu = std::make_unique<morph::GraphVisual<float>> (morph::vec<float>({0,1.1,0})+izoff);
    v.bindmodel (gu);
    gu->twodimensional = twodee;
    gu->setsize (1,0.5);
    gu->xlabel = "t";
    gu->ylabel = "u";
    ds.datalabel = "u(t)";
    ds.markercolour = morph::colour::crimson;
    ds.linecolour = morph::colour::crimson;
    ds.markerstyle = morph::markerstyle::uphexagon;
    gu->setdata (t, _u, ds);
    gu->addLabel ("using morph::stylepolicy::both\nand morph::markerstyle::diamond", morph::vec<float>({0.3,0.6,0}), morph::TextFeatures(0.05));
    gu->finalize();
    v.addVisualModel (gu);

    // Graph nullclines, u vs v and vector field
    ds.showlines = false;
    auto gp = std::make_unique<morph::GraphVisual<float>> (morph::vec<float>({1.5,0,0})+izoff);
    v.bindmodel (gp);
    gp->twodimensional = twodee;
    gp->setsize (1.6, 1.6);
    gp->xlabel = "v";
    gp->ylabel = "u";
    // nullcline for the u variable
    ds.markercolour = morph::colour::crimson;
    ds.datalabel = "u nc";
    gp->setdata (vrng, u_nc, ds);
    // nullcline for the v variable
    ds.markercolour = morph::colour::royalblue;
    ds.datalabel = "v nc";
    gp->setdata (vrng, v_nc, ds);
    // The evolution of v and u wrt time
    ds.markercolour = morph::colour::black;
    ds.datalabel = "u(v)";
    gp->setdata (_v, _u, ds);
    // Plot quivs within graphvisual
    ds.datalabel = "quivs";
    ds.quiver_gain = { 0.08f, 0.8f, 1.0f };
    gp->quiver_setlog();
    ds.quiver_colourmap.setType (morph::ColourMapType::Jet);
    ds.quiver_conewidth = 1.8f;
    //ds.quiver_thickness_gain = 0.6f; // make arrows a bit thinner
    ds.markerstyle = morph::markerstyle::quiver;
    gp->setdata (grid, du_dv_vecfield, ds);
    gp->finalize();
    gp->addLabel ("using morph::markerstyle::quiver", morph::vec<float>({0,-0.25,0}), morph::TextFeatures(0.05));
    v.addVisualModel (gp);

    /*
     * We have a loop to update dynamic elements. Within this loop, we regularly call
     * v.render().
     */
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

        // Re-init the dynamic scatter visual
        svp->reinit();

        v.wait (0.008);
        v.render();
    }

    return 0;
}
