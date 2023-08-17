// Graph the logistic function
#include <morph/Visual.h>
#include <morph/GraphVisual.h>
#include <morph/vvec.h>
#include <sstream>
#include <morph/Config.h>

int main()
{
    // We'll use morphologica's awesome unicode chars
    using morph::unicode;
    // Set up a morph::Visual 'scene environment'.
    morph::Visual v(1024, 768, "Logistic functions");
    v.addLabel ("Change logistic function parameters in ../examples/graph_logist2.json", morph::vec<float>({0,0,0}));
    // Create a GraphVisual object (obtaining a unique_ptr to the object) with a spatial offset within the scene of 0,0,0
    auto gv = std::make_unique<morph::GraphVisual<double>> (morph::vec<float>({-0.5f,-0.5f,0.0f}));
    v.bindmodel (gv);
    // Params are read from a JSON file
    morph::Config conf ("../examples/graph_logist2.json");
    double ofst = conf.get<double> ("ofst", 4.0);
    double alpha = conf.get<double> ("alpha", 10.0);
    double x0 = conf.get<double> ("x0", -10.0);
    double x1 = conf.get<double> ("x1", 10.0);
    double m = conf.get<double> ("m", 12.0);
    // Data for the x axis. A vvec is like std::vector, but with built-in maths methods
    morph::vvec<double> x;
    // This works like numpy's linspace() (the 3 args are "start", "end" and "num"):
    x.linspace (x0, x1, 100);
    // Logistic functions. Args are parameters to the function are (xoffset, alpha)
    std::stringstream lftag;
    lftag << "ofst=" << ofst << ", " << unicode::toUtf8(unicode::alpha) << "=" << alpha; // A dataset tag
    // vvec::logistic() returns a new vvec with the logistic function-transformed values:
    gv->setdata (x, (x*m).logistic(ofst, alpha), lftag.str());

    // finalize() makes the GraphVisual compute the vertices of the OpenGL model
    gv->finalize();
    // Add the GraphVisual OpenGL model to the Visual scene, transferring ownership of the unique_ptr
    v.addVisualModel (gv);

    auto gv2 = std::make_unique<morph::GraphVisual<double>> (morph::vec<float>({1.0f,-0.5f,0.0f}));
    v.bindmodel (gv2);
    morph::vvec<double> x2;
    x2.linspace (0, 1, 100);
    gv2->setlimits (0,1,0,1);
    gv2->setdata (x2, (x2*m).logistic(ofst, alpha), lftag.str());
    gv2->finalize();
    v.addVisualModel (gv2);

    // Render the scene on the screen until user quits with 'Ctrl-q'
    v.keepOpen();
    return 0;
}
