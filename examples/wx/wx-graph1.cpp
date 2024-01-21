#include <wx/wx.h>

#include <GL/glew.h> // must be included before glcanvas.h
#include <wx/glcanvas.h>

#include <wx/colordlg.h>

#include <morph/wx/viswx.h>

#include <morph/GraphVisual.h>
#include <morph/vvec.h>

// Here, we specialise a morph::wx::Canvas, introducing the VisualModels that will be displayed in
// the canvas in the overridden function setupVisualModels.
template<int glver>
struct MyCanvas : public morph::wx::Canvas<glver>
{
    // Call parent constructor
    MyCanvas(wxFrame* parent, const wxGLAttributes& canvasAttrs)
        : morph::wx::Canvas<glver>(parent, canvasAttrs) {}

    // This function, which sets up the morph::VisualModels, is called when GL is initialized.
    void setupVisualModels()
    {
        if (this->ready()) {
            // We can now add VisualModels to the Visual inside the Widget. Create a GraphVisual
            // object (obtaining a unique_ptr to the object) with a spatial offset within the
            // scene of 0,0,0
            auto gv = std::make_unique<morph::GraphVisual<double, glver>> (morph::vec<float>({0,0,0}));
            // This mandatory line of boilerplate code sets the parent pointer in GraphVisual and binds some functions
            this->v.bindmodel (gv);
            // Allow 3D
            gv->twodimensional = false;
            // Data for the x axis. A vvec is like std::vector, but with built-in maths methods
            morph::vvec<double> x;
            // This works like numpy's linspace() (the 3 args are "start", "end" and "num"):
            x.linspace (-0.5, 0.8, 14);
            // Set a graph up of y = x^3
            gv->setdata (x, x.pow(3));
            // finalize() makes the GraphVisual compute the vertices of the OpenGL model
            gv->finalize(); // Requires opengl context
            // Add the GraphVisual OpenGL model to the Visual scene, transferring ownership of the unique_ptr
            std::cout << "add visualmodel to my morph::wx::Canvas" << std::endl;
            this->v.addVisualModel (gv);

        } else {
            // Not ready
            throw std::runtime_error ("Canvas is not ready (no gl context yet)");
        }
    }
};


// Choose an OpenGL version and pass this as a template argument to your MyFrame. version_4_1 is the
// default across morphologica. version_3_1_es allows your code to run on small ARM devices such as
// a Raspberry Pi.
constexpr int gl_version = morph::gl::version_4_1;

// Your application-specific frame, deriving from morph::wx:Frame. In this frame, I'll set up VisualModels
class MyFrame : public wxFrame
{
public:
    MyFrame(const wxString &title)
        : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxDefaultSize)
    {
        // First job, set up a new morph::wx::Canvas
        wxGLAttributes vAttrs;
        vAttrs.PlatformDefaults().Defaults().EndList();
        if (wxGLCanvas::IsDisplaySupported(vAttrs)) {
            // canvas becomes a child of this wxFrame which is responsible for deallocation
            this->canvas = new MyCanvas<gl_version>(this, vAttrs);
            this->canvas->SetMinSize (FromDIP (wxSize(640, 480)));
        } else {
            throw std::runtime_error ("wxGLCanvas::IsDisplaySupported(vAttrs) returned false");
        }

        auto sizer = new wxBoxSizer(wxVERTICAL);

        // Adding the GL canvas, where all the morphologica stuff will be drawn
        sizer->Add (this->canvas, 1, wxEXPAND);

        SetSizerAndFit(sizer);

    }

    // Your Frame must contain a morph::wx::Canvas-derived canvas class
    MyCanvas<gl_version>* canvas;
};

// Your app, containing your frame
class MyApp : public wxApp
{
public:
    MyApp() {}
    bool OnInit();
};

bool MyApp::OnInit()
{
    if (!wxApp::OnInit()) { return false; }
    MyFrame *frame = new MyFrame("Hello OpenGL");
    frame->Show(true);
    return true;
}

// This macro implements an int main(){} stanza in a Windows and Unix compatible way
wxIMPLEMENT_APP(MyApp);

#if 0
// The equivalent which works fine on Linux is:
int main (int argc, char* argv[])
{
    wxApp::SetInstance(new MyApp{});
    return wxEntry(argc, argv);
}
#endif
