/*
 * Compute a single Izhikevich neuron model and plot
 */

#include <morph/vvec.h>
#include <morph/Visual.h>
#include <morph/GraphVisual.h>

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

    // Apply one timestep of the differential equations for the model
    void step()
    {
        float dv = AT * v * v + BT * v + CT - u * T + I * ToverSI;
        float du = a * T * (b * v - u);
        bool spike = (v > vpeak); // This is the reset condition
        v = spike ? c : (v+dv);
        u = spike ? (u+d) : (u+du);
    }

    // Compute nullclines. For Vn, the given input membrane voltages, return u and v nullclines in u_nc and v_nc
    void nullclines (const morph::vvec<float>& Vn, morph::vvec<float>& u_nc,  morph::vvec<float>& v_nc)
    {
        //Vn = np.linspace (lowerV, upperV, 1000);
        u_nc.resize (Vn.size(), 0.0f);
        v_nc.resize (Vn.size(), 0.0f);
        for (unsigned int i = 0; i < Vn.size(); ++i) {
            v_nc[i] = A * Vn[i] * Vn[i] + B * Vn[i] + C + I / SI;
            u_nc[i] = Vn[i] * b;
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
     * Visualize results
     */

    morph::Visual vis(1024, 768, "Izhikevich Neuron Model");

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

    gp->finalize();
    vis.addVisualModel (gp);

    vis.keepOpen();

    return 0;
}
