#ifdef USE_GLEW
# include <gl/glew.h>
#endif
#include <wx/glcanvas.h>
#include <wx/wx.h>

// Note sure why the DELETE macro got conflict with this library. Seb: Possibly because of
// morph::key::DELETE. DELETE must be defined in wx somewhere and conflicts with morph::key::DELETE?
// Not sure of best fix, but leave this for now.
#ifdef DELETE
    #undef DELETE
#endif // DELETE

#include <morph/wx/viswidget.h>
#include <morph/GraphVisual.h>
#include <morph/vvec.h>

// Define the custom frame class
class MyFrame : public wxFrame
{
public:
    MyFrame() : wxFrame(nullptr, wxID_ANY, "Simple wxWidgets Program")
    {
        wxGLAttributes vAttrs;
        // Defaults should be accepted
        vAttrs.PlatformDefaults().Defaults().EndList();
        bool accepted = wxGLCanvas::IsDisplaySupported(vAttrs);
        if (!accepted) {
            std::cout << "Warning: wxGLCAnvas::IsDisplaySupported returned false\n";
        }
        // Create the viswidget as a child of the frame
        widget = new morph::wx::viswidget(this, vAttrs);

        // Set up the layout
        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
        sizer->Add(widget, 1, wxEXPAND);

        SetSizer(sizer);

        // We can now add VisualModels to the Visual inside the Widget. Create a GraphVisual
        // object (obtaining a unique_ptr to the object) with a spatial offset within the
        // scene of 0,0,0
        auto gv = std::make_unique<morph::GraphVisual<double>> (morph::vec<float>({0,0,0}));
        // This mandatory line of boilerplate code sets the parent pointer in GraphVisual and binds some functions
        widget->v.bindmodel (gv);
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
        widget->v.addVisualModel (gv);
    }

private:
    morph::wx::viswidget* widget;
};

// Define the application class
class MyApp : public wxApp
{
public:
    virtual bool OnInit()
    {
        // Create the main frame
        MyFrame* frame = new MyFrame();
        frame->Show();

        return true;
    }
};

// Start the application
wxIMPLEMENT_APP(MyApp);
