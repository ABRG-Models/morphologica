// Graph the logistic function
#include <morph/Visual.h>
#include <morph/GraphVisual.h>
#include <morph/vvec.h>
#include <morph/unicode.h>

// Make an equation string for the legend
std::string make_legend_str (double k, double x0)
{
    // We'll use morphologica's awesome unicode chars for the subscript 0 on x0
    namespace uc = morph::unicode;

    std::stringstream ktxt;
    if (k != 1.0) { ktxt << k; }
    std::stringstream brtxt;
    std::stringstream ostxt;
    if (x0 != 0.0) {
        brtxt << "(";
        if (x0 > 0.0) {
            ostxt << " - " << x0 <<")";
        } else {
            ostxt << " + " << -x0 <<")";
        }
    }
    std::stringstream eqn;
    eqn << "k="<<k<<", x" << uc::toUtf8(uc::subs0) << "=" << x0;
    eqn << ": f(x) = 1 / [1 + exp (-"<< ktxt.str() << brtxt.str() << "x" << ostxt.str() << ")]";
    return eqn.str();
}

int main()
{
    // Set up a morph::Visual 'scene environment'.
    morph::Visual v(1024, 768, "Logistic functions");
    // Create a GraphVisual object (obtaining a unique_ptr to the object) with a spatial offset within the scene of 0,0,0
    auto gv = std::make_unique<morph::GraphVisual<double>> (morph::vec<float>({-0.5f,-0.5f,0.0f}));
    v.bindmodel (gv);
    // Data for the x axis. A vvec is like std::vector, but with built-in maths methods
    morph::vvec<double> x;
    // This works like numpy's linspace() (the 3 args are "start", "end" and "num"):
    x.linspace (-100, 30, 200);
    // Logistic functions. Args are parameters to the function: (k, x0)
    // vvec::logistic() returns a new vvec with the logistic function-transformed values:
    gv->setdata (x, x.logistic(0.1, -10), make_legend_str (0.1, -10));
    // For this one, demonstrate use of logistic_inplace():
    morph::vvec<double> xlogistic = x;
    xlogistic.logistic_inplace(0.25, -5);
    gv->setdata (x, xlogistic, make_legend_str (0.25, -5));
    gv->setdata (x, x.logistic(0.5, 0), make_legend_str (0.5, 0));
    gv->setdata (x, x.logistic(1, 5), make_legend_str (1, 5));
    gv->setdata (x, x.logistic(2, 10), make_legend_str (2, 10));
    // finalize() makes the GraphVisual compute the vertices of the OpenGL model
    gv->finalize();
    // Add the GraphVisual OpenGL model to the Visual scene, transferring ownership of the unique_ptr
    v.addVisualModel (gv);
    // Render the scene on the screen until user quits with 'Ctrl-q'
    v.keepOpen();
    return 0;
}
