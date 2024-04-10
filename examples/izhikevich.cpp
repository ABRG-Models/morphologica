/*
 * Compute a single Izhikevich neuron model and plot
 */

#include <morph/vvec.h>
#include <morph/Visual.h>
#include <morph/GraphVisual.h>
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
    constexpr bool twodee = false;

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
    // Now plot with a Grid and a GraphVisual? Or initially with a QuiverVisual
    morph::vec<float, 2> gridspacing = {
        (vmax - vmin) / (qN-1),
        (umax - umin) / (qN-1)
    };
    morph::vec<float, 2> gridzero = { vmin, umin };
    morph::Grid<unsigned int, float> grid (qN, qN, gridspacing, gridzero);

    /*
     * Visualize results
     */

    morph::Visual vis(1280, 768, "Izhikevich Neuron Model");
    vis.setSceneTrans (morph::vec<float,3>({-0.877793f, -0.281277f, -3.9f}));

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
    gv->twodimensional = twodee;
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
    gu->twodimensional = twodee;
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
    gp->twodimensional = twodee;
    gp->setsize (1.6,1.6);
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
    gp->setdata (v, u, ds);

    // Plot quivs within graphvisual
    ds.datalabel = "quivs";
    // Set a linear gain to apply to the quivers...
    ds.quiver_gain = { 0.08f, 0.8f, 1.0f };
    // ...and then if the lengths should be log-scaled, call quiver_setlog()
    gp->quiver_setlog();
    ds.quiver_colourmap.setType (morph::ColourMapType::Jet);
    ds.quiver_conewidth = 1.3f;
    ds.quiver_thickness_gain = 0.6f; // make arrows a bit thinner
    ds.markerstyle = morph::markerstyle::quiver;
    //ds.quiver_flagset.set (static_cast<unsigned int>(morph::quiver_flags::show_zeros));
    //ds.quiver_flagset.set (static_cast<unsigned int>(morph::quiver_flags::marker_sphere));
    gp->setdata (grid, du_dv_vecfield, ds);
    gp->finalize();
    vis.addVisualModel (gp);

    // Keep showing graphs until user exits
    vis.keepOpen();

    return 0;
}
