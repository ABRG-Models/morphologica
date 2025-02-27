#include <QtWidgets/QApplication>
#include <morph/qt/viswidget.h>
#include <morph/GraphVisual.h>
#include <morph/vvec.h>

// Build a widget based morph app
int main (int argc, char **argv)
{
    QApplication app(argc, argv);

    // Create widget. The GL version to be used is set inside viswidget. It's accessible as
    // morph::qt::gl_version. You have to give an index to your widget (0 for the first, 1 for the next one, etc)
    morph::qt::viswidget<0> widget;
    // Calling show ensures initializeGL() method gets called. The alternative to
    // calling show() at the start of the main() function, is to set viswidget's
    // buildmodels callback (see app2).
    widget.show();

    // We can now add VisualModels to the Visual inside the Widget. Create a GraphVisual
    // object (obtaining a unique_ptr to the object) with a spatial offset within the
    // scene of 0,0,0
    auto gv = std::make_unique<morph::GraphVisual<double, morph::qt::gl_version>> (morph::vec<float>({0,0,0}));
    // This mandatory line of boilerplate code sets the parent pointer in GraphVisual and binds some functions
    widget.v.bindmodel (gv);
    // Allow 3D
    gv->twodimensional = false;
    // Data for the x axis. A vvec is like std::vector, but with built-in maths methods
    morph::vvec<double> x;
    // This works like numpy's linspace() (the 3 args are "start", "end" and "num"):
    x.linspace (-0.5, 0.8, 14);
    // Set a graph up of y = x^3
    gv->setdata (x, x.pow(3));
    // finalize() makes the GraphVisual compute the vertices of the OpenGL model
    gv->finalize();
    // Add the GraphVisual OpenGL model to the Visual scene, transferring ownership of the unique_ptr
    widget.v.addVisualModel (gv);

    return app.exec();
}
