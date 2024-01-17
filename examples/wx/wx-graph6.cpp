#include <wx/wx.h>

#include <GL/glew.h> // must be included before glcanvas.h
#include <wx/glcanvas.h>

#include <wx/colordlg.h>

#include <morph/wx/viswx.h>

#include <morph/GraphVisual.h>
#include <morph/mathconst.h>
#include <morph/vvec.h>

// Your application-specific frame, deriving from morph::wx:Frame. In this frame, I'll set up VisualModels
class MyFrame : public morph::wx::Frame
{
public:
    MyFrame(const wxString &title) : morph::wx::Frame(title)
    {
        auto sizer = new wxBoxSizer(wxVERTICAL);

        // Adding the GL canvas, where all the morphologica stuff will be drawn
        sizer->Add (this->canvas, 1, wxEXPAND);

        auto bottomSizer = new wxBoxSizer(wxHORIZONTAL);
        auto colorButton = new wxButton(this, wxID_ANY, "Change Color");

        bottomSizer->Add(colorButton, 0, wxALL | wxALIGN_CENTER, FromDIP(15));
        bottomSizer->AddStretchSpacer(1);

        sizer->Add(bottomSizer, 0, wxEXPAND);

        SetSizerAndFit(sizer);

        colorButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent &event) {
            wxColourData colorData;
            wxColourDialog dialog(this, &colorData);
            if (dialog.ShowModal() == wxID_OK) {
                this->canvas->Refresh();
            }
        }); // end of lambda
    }

    // Data for the x axis. A vvec is like std::vector, but with built-in maths methods
    morph::vvec<double> x;
    // Member variable used to update GraphVisual
    double dx = 0.0;
    // A pointer to our GraphVisual
    morph::GraphVisual<double> graph_ptr = nullptr;

    // A function to make a dynamic update to the model
    void updateModel()
    {
        this->dx += 0.01;
        this->graph_ptr->update (this->x, (this->x+this->dx).sin(), 0);
        // Now we mark that VisualModel 0 (the first and only VisualModel that we added in
        // setupVisualModels) nees to be reinitialized.
        this->canvas->needs_reinit = 0;
    }

    // To set up the VisualModels in the widget, the GL context must have been initialized. So I'll
    // have to have a callback in viswidget (soon to be viscanvas) which will call this function
    // after gl init has completed.
    void setupVisualModels()
    {
        if (this->canvas->ready()) {
            // We can now add VisualModels to the Visual inside the Widget. Create a GraphVisual
            // object (obtaining a unique_ptr to the object) with a spatial offset within the
            // scene of 0,0,0
            auto gv = std::make_unique<morph::GraphVisual<double>> (morph::vec<float>({0,0,0}));
            // This mandatory line of boilerplate code sets the parent pointer in GraphVisual and binds some functions
            this->canvas->v.bindmodel (gv);
            // Allow 3D
            gv->twodimensional = false;
            // This works like numpy's linspace() (the 3 args are "start", "end" and "num"):
            this->x.linspace (-morph::mathconst<double>::pi, morph::mathconst<double>::pi, 100);

            // Set a graph up of y = x^3
            gv->setdata (this->x, (this->x+this->dx).sin());

            // finalize() makes the GraphVisual compute the vertices of the OpenGL model
            gv->finalize(); // Requires opengl context

            // Add the GraphVisual OpenGL model to the Visual scene, transferring ownership of the unique_ptr
            std::cout << "add visualmodel to morph::wx::Canvas" << std::endl;

            this->graph_ptr = this->canvas->v.addVisualModel (gv);

        } else {
            // Not ready
            throw std::runtime_error ("Canvas is not ready (no gl context yet)");
        }
    }

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
    frame->setupVisualModels(); // After calling Show()
    // put updateModel() on a timer.
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
