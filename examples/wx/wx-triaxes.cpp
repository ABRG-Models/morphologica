#include <wx/wx.h>

#include <GL/glew.h> // must be included before glcanvas.h
#include <wx/glcanvas.h>

#include <wx/colordlg.h>

#include <morph/gl/version.h>
#include <morph/wx/viswx.h>
#include <morph/TriaxesVisual.h>

constexpr int gl_version = morph::gl::version_4_1; // options in morph/gl/version.h

// Your application-specific frame, deriving from morph::wx:Frame. In this frame, I'll set up VisualModels
class MyFrame : public morph::wx::Frame<gl_version>
{
public:
    MyFrame(const wxString &title) : morph::wx::Frame<gl_version>(title)
    {
        auto sizer = new wxBoxSizer(wxVERTICAL);
        // Adding ONLY the GL canvas, where all the morphologica stuff will be drawn
        sizer->Add (this->canvas, 1, wxEXPAND);
        SetSizerAndFit(sizer);
    }

    // To set up the VisualModels in the widget, the GL context must have been initialized. So I'll
    // have to have a callback in viswidget (soon to be viscanvas) which will call this function
    // after gl init has completed.
    void setupVisualModels()
    {
        if (this->canvas->ready()) {
            auto tav = std::make_unique<morph::TriaxesVisual<float, gl_version>> (morph::vec<float,3>({0,0,0}));
            this->canvas->v.bindmodel (tav);
            tav->axisstyle = morph::axisstyle::L;
            // Specify axes min and max with a min and max vector
            //                                         x      y       z
            tav->input_min = morph::vec<float, 3>({ -1.0f,  0.0f,   0.0f });
            tav->input_max = morph::vec<float, 3>({  1.0f, 10.0f, 100.0f });
            // Set the axis labels
            tav->xlabel = "x";
            tav->ylabel = "y";
            tav->zlabel = "z";
            tav->finalize();
            this->canvas->v.addVisualModel (tav);
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
    MyFrame *frame = new MyFrame("morph::TriaxesVisual");
    frame->Show(true);
    frame->setupVisualModels(); // After calling Show()
    return true;
}

wxIMPLEMENT_APP(MyApp);
