/*
 * Compute a single Izhikevich neuron model and plot. This uses the formulation of the model given
 * in Izhikevich, Dynamical Systems in Neuroscience p 273, Eqs. 8.5 and 8.6.
 */

#include <morph/vvec.h>
#include <morph/Visual.h>
#include <morph/GraphVisual.h>
#include <morph/Grid.h>
#include <morph/Config.h>

// A simple Izhikevich neuron model class
struct izhi
{
    // state variables
    float I = 0.0f;      // Input current
    float u = 0.0f;      // 'The refractory variable'
    float v = -60.0f;    // 'Membrane voltage'

    // Parameters. These values are from the book, but the json will specify replacements
    float a = 0.03f;
    float b = -2.0f;
    float c = -50.0f;
    float d = 100.0f;

    float k = 0.7f;
    float vr = -60.0f;
    float vt = -40.0f;

    float vpeak = 35.0f;

    float CC = 100.0f; // rather than C, to distinguish from ABC

    // vdot and udot computations
    float dv (const float _u, const float _v)
    {
        float Cvdot = k * (_v - vr) * (_v - vt) - _u + I;
        return Cvdot / CC;
    }

    //float du (const float _u, const float _v) { return a * T * (b * _v - _u); }
    float du (const float _u, const float _v) { return a * (b * (_v - vr) - _u); }

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
            v_nc[i] = k * (Vn[i] - vr) * (Vn[i] - vt) + I;
            u_nc[i] = (Vn[i] - vr) * b ;
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


int main (int argc, char** argv)
{
    constexpr unsigned int N = 1000; // How many steps to run
    constexpr bool twodee = false;   // Should graphs be 2D only or rotatable?

    /*
     * Perform simulation
     */

    // Create class and vars
    morph::vvec<float> u(N, 0.0f);
    morph::vvec<float> v(N, 0.0f);
    izhi iz;

    // Set izhi params from config
    std::string jsonfile ("../examples/izhikevich_alt.json");

    bool user_file = false;
    if (argc > 1) {
        jsonfile = std::string (argv[1]);
        user_file = true;
    }

    std::string title = "Izhikevich Neuron Model";
    morph::Config config(jsonfile);
    if (config.ready) {
        // Parameters
        iz.a = config.getFloat ("a", iz.a);
        iz.b = config.getFloat ("b", iz.b);
        iz.c = config.getFloat ("c", iz.c);
        iz.d = config.getFloat ("d", iz.d);
        iz.k = config.getFloat ("k", iz.k);
        iz.vr = config.getFloat ("vr", iz.vr);
        iz.vt = config.getFloat ("vt", iz.vt);
        iz.vpeak = config.getFloat ("vpeak", iz.vpeak);
        iz.CC = config.getFloat ("CC", iz.CC);

        // Initial values of state vars
        iz.u = config.getFloat ("u0", iz.u);
        iz.v = config.getFloat ("v0", iz.v);

        iz.I = config.getFloat ("I", iz.I);

        title = config.getString ("description", title);
    } else if (!config.ready && user_file == true) {
        throw std::runtime_error ("Failed to open json file given by user");
    } // else use defaults

    std::cout << "Model parameters:\na/b/c/d: " << iz.a << "/" << iz.b << "/" << iz.c << "/" << iz.d
              << "\nC=" << iz.CC << " vr="  << iz.vr << " vt=" << iz.vt << " k=" << iz.k << ", vpeak=" << iz.vpeak << std::endl;
    std::cout << "Initial state: v=" << iz.v << ", u=" << iz.u << " with I=" << iz.I << std::endl;

    // Run sim
    for (unsigned int i = 0; i < N; ++i) {
        iz.step();
        v[i] = iz.v;
        u[i] = iz.u;
    }
    // Find range of the state variables u and v for plotting
    morph::range<float> v_range = v.range();
    if (v_range.max > iz.vpeak) { v_range.max = iz.vpeak; }
    morph::range<float> u_range = u.range();

    /*
     * Compute nullclines
     */

    morph::vvec<float> u_nc;
    morph::vvec<float> v_nc;
    morph::vvec<float> vrng;
    vrng.linspace (-70.0f, iz.vpeak, N);
    iz.nullclines (vrng, u_nc, v_nc);

    /*
     * Compute du/dv vector field
     */

    static constexpr size_t qN = 50;
    morph::vvec<float> qurng; // y axis
    morph::vvec<float> qvrng; // x axis
    qvrng.linspace (v_range.min, v_range.max, qN);
    qurng.linspace (u_range.min, u_range.max, qN);
    morph::vvec<morph::vec<float, 2>> du_dv_vecfield;
    iz.vectorfield (qurng, qvrng, du_dv_vecfield);
    // Now plot with a Grid and a GraphVisual? Or initially with a QuiverVisual
    morph::vec<float, 2> gridspacing = {
        (v_range.span()) / (qN-1),
        (u_range.span()) / (qN-1)
    };
    morph::vec<float, 2> gridzero = { v_range.min, u_range.min };
    morph::Grid<unsigned int, float> grid (qN, qN, gridspacing, gridzero);

    /*
     * Visualize results
     */

    morph::Visual vis(1280, 768, title);
    vis.setSceneTrans (morph::vec<float,3>({-0.877793f, -0.281277f, -3.9f}));
    vis.lightingEffects();

    // Time
    morph::vvec<float> t(N, 0.0f);
    t.linspace (0.0f, N-1, N);

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
    // Fix plotting range
    gp->setlimits_x (v_range);
    gp->setlimits_y (u_range);
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
    ds.quiver_gain = { 0.01f, 0.1f, 1.0f };
    // ...and then if the lengths should be log-scaled, call quiver_setlog()
    gp->quiver_setlog();
    ds.quiver_colourmap.setType (morph::ColourMapType::Jet);
    ds.quiver_conewidth = 1.8f;
    ds.quiver_arrowhead_prop = 0.35f;
    ds.quiver_thickness_gain = 1.5f;
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
