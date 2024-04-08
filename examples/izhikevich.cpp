/*
 * Compute a single Izhikevich neuron model and plot
 */

#include <morph/vvec.h>
#include <morph/Visual.h>
#include <morph/GraphVisual.h>
#include <morph/QuiverVisual.h>
#include <morph/Grid.h>

// A simple Izhikevich neuron model class
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
};


int main()
{
    constexpr unsigned int N = 1000;

    /*
     * Perform simulation
     */

    morph::vvec<float> u(N, 0.0f);
    morph::vvec<float> v(N, 0.0f);
    izhi iz;
    for (unsigned int i = 0; i < N; ++i) {
        iz.step();
        v[i] = iz.v;
        u[i] = iz.u;
    }

    /*
     * Compute nullclines
     */

    morph::vvec<float> u_nc;
    morph::vvec<float> v_nc;
    morph::vvec<float> vrng;
    vrng.linspace (-80.0f, -20.0f, 1000);
    iz.nullclines (vrng, u_nc, v_nc);

    /*
     * Compute du/dv vector field
     */

    static constexpr size_t qN = 50;
    morph::vvec<float> qurng; // y axis
    morph::vvec<float> qvrng; // x axis
    qvrng.linspace (-80.0f, -20.0f, qN);
    qurng.linspace (-16, -4, qN);
    morph::vvec<morph::vec<float, 2>> quivs;
    iz.vectorfield (qurng, qvrng, quivs);
    std::cout << "quivs size: " << quivs.size() << std::endl;
    // Now plot with a Grid and a GraphVisual? Or initially with a QuiverVisual
    morph::vec<float, 2> gridspacing = {
        (-20.0f - (-80.0f)) / (qN-1),
        (-4.0f - (-16.0f)) / (qN-1)
    };
    morph::vec<float, 2> gridzero = { -80.0f, -16.0f };
    std::cout << "Grid spacing: " << gridspacing << std::endl;
    morph::Grid<unsigned int, float> grid (50, 50, gridspacing, gridzero);

    /*
     * Visualize results
     */

    morph::Visual vis(1920, 768, "Izhikevich Neuron Model");
    vis.setSceneTrans (morph::vec<float,3>({-1.80045f, -0.28672f, -3.9f}));

    // Time
    morph::vvec<float> t(N, 0.0f);
    t.linspace (0.0f, N/100.0f, N);

    // Set default dataset graphing styles
    morph::DatasetStyle ds;
    ds.linewidth = 0.003f;
    ds.linecolour = morph::colour::grey30;
    ds.markersize = 0.015f;
    ds.markerstyle = morph::markerstyle::uphexagon;

    // Graph membrane voltage vs. time
    auto gv = std::make_unique<morph::GraphVisual<float>> (morph::vec<float>({-0.5,-0.5,0}));
    vis.bindmodel (gv);
    gv->setsize (1,0.8);
    gv->xlabel = "t";
    gv->ylabel = "v";
    ds.datalabel = "v(t)";
    gv->setdata (t, v, ds);
    gv->finalize();
    vis.addVisualModel (gv);

    // Graph u(t)
    auto gu = std::make_unique<morph::GraphVisual<float>> (morph::vec<float>({-0.5,0.6,0}));
    vis.bindmodel (gu);
    gu->setsize (1,0.5);
    gu->xlabel = "t";
    gu->ylabel = "u";
    ds.datalabel = "u(t)";
    ds.markercolour = morph::colour::crimson;
    gu->setdata (t, u, ds);
    gu->finalize();
    vis.addVisualModel (gu);

    // Graph nullclines, u vs v and vector field
    ds.showlines = false;
    auto gp = std::make_unique<morph::GraphVisual<float>> (morph::vec<float>({0.9,-0.5,0}));
    vis.bindmodel (gp);
    gp->setsize (1.6,1.6);
    gp->xlabel = "v";
    gp->ylabel = "u";

    ds.markercolour = morph::colour::crimson;
    ds.datalabel = "u nc";
    gp->setdata (vrng, u_nc, ds);

    ds.markercolour = morph::colour::royalblue;
    ds.datalabel = "v nc";
    gp->setdata (vrng, v_nc, ds);

    ds.markercolour = morph::colour::springgreen;
    ds.datalabel = "u(v)";
    gp->setdata (v, u, ds);

    ds.datalabel = "quiv";
    ds.markerstyle = morph::markerstyle::quiver;
    gp->setdata (grid, quivs, ds);

    gp->finalize();
    vis.addVisualModel (gp);

    // Plot quivs with a QuiverVisual, just to see them
    std::vector<morph::vec<float, 3>> coords(quivs.size());
    for (unsigned int j = 0; j < qurng.size(); ++j) {
        unsigned int shft = j * qvrng.size();
        for (unsigned int i = 0; i < qvrng.size(); ++i) {
            // Note scaling of coord for QuiverVisual:
            coords[i + shft] = {2.0f*qvrng[i]/80.0f, 2.0f*qurng[j]/16.0f, 0.0f};
        }
    }
    std::vector<morph::vec<float, 3>> quiv3s(quivs.size());
    for (unsigned int i = 0; i < quivs.size(); ++i) {
        quiv3s[i][0] = quivs[i][0];
        quiv3s[i][1] = quivs[i][1] * 10.0f; // arb scaling of du magnitude
    }
    auto vmp = std::make_unique<morph::QuiverVisual<float>>(&coords, morph::vec<float>({4.8,1.5,0}),
                                                            &quiv3s, morph::ColourMapType::Jet);
    vis.bindmodel (vmp);
    vmp->twodimensional = true;
    vmp->quiver_length_gain = 0.08f; // Scale the length of the quivers on screen
    vmp->quiver_thickness_gain = 0.05f; // Scale thickness of the quivers
    vmp->qgoes = morph::QuiverGoes::OnCoord;
    vmp->show_coordinate_sphere = false;
    vmp->shapesides = 12;
    vmp->colourScale.compute_autoscale (0.01f, 5.0f);
    vmp->setlog();
    vmp->finalize();
    vis.addVisualModel (vmp);

    // Keep showing graphs until user exits
    vis.keepOpen();

    return 0;
}
