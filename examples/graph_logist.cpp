// Graph the logistic function
#include <morph/Visual.h>
#include <morph/GraphVisual.h>
#include <morph/vvec.h>

int main()
{
    // We'll use morphologica's awesome unicode chars
    using morph::unicode;
    // Set up a morph::Visual 'scene environment'.
    morph::Visual v(1024, 768, "Logistic functions");
    // Create a GraphVisual object (obtaining a unique_ptr to the object) with a spatial offset within the scene of 0,0,0
    auto gv = std::make_unique<morph::GraphVisual<double>> (v.shaders, morph::vec<float>({-0.5f,-0.5f,0.0f}));
    // Data for the x axis. A vvec is like std::vector, but with built-in maths methods
    morph::vvec<double> x;
    // This works like numpy's linspace() (the 3 args are "start", "end" and "num"):
    x.linspace (-100, 30, 200);
    // Logistic functions. Args are parameters to the function are (xoffset, alpha)
    std::string lftag = std::string("ofst=-10, ") + unicode::toUtf8(unicode::alpha) + std::string("=0.1"); // A dataset tag
    // vvec::logistic() returns a new vvec with the logistic function-transformed values:
    gv->setdata (x, x.logistic(-10, 0.1), lftag);
    // For this one, demonstrate use of logistic_inplace():
    lftag = std::string("ofst=-5, ") + unicode::toUtf8(unicode::alpha) + std::string("=0.25");
    morph::vvec<double> xlogistic = x;
    xlogistic.logistic_inplace(-5, 0.25);
    gv->setdata (x, xlogistic, lftag);
    lftag = std::string("ofst=0, ") + unicode::toUtf8(unicode::alpha) + std::string("=0.5");
    gv->setdata (x, x.logistic(0, 0.5), lftag);
    lftag = std::string("ofst=5, ") + unicode::toUtf8(unicode::alpha) + std::string("=1");
    gv->setdata (x, x.logistic(5, 1), lftag);
    lftag = std::string("ofst=10, ") + unicode::toUtf8(unicode::alpha) + std::string("=2");
    gv->setdata (x, x.logistic(10, 2), lftag);
    // finalize() makes the GraphVisual compute the vertices of the OpenGL model
    gv->finalize();
    // Add the GraphVisual OpenGL model to the Visual scene, transferring ownership of the unique_ptr
    v.addVisualModel (gv);
    // Render the scene on the screen until user quits with 'x'
    v.keepOpen();
    return 0;
}
