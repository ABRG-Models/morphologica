#include <wx/wx.h>

#include <GL/glew.h> // must be included before glcanvas.h
#include <wx/glcanvas.h>

#include <wx/colordlg.h>

#include <morph/wx/viswx.h>

// Your application-specific frame, deriving from morph::wx:Frame.
class MyFrame : public morph::wx::Frame
{
public:
    MyFrame(const wxString &title)
        : morph::wx::Frame(title)
    {
        auto sizer = new wxBoxSizer(wxVERTICAL);

        // Adding the GL canvas, where all the morphologica stuff will be drawn
        sizer->Add (this->canvas.get(), 1, wxEXPAND);

        auto bottomSizer = new wxBoxSizer(wxHORIZONTAL);
        auto colorButton = new wxButton(this, wxID_ANY, "Change Color");

        bottomSizer->Add(colorButton, 0, wxALL | wxALIGN_CENTER, FromDIP(15));
        bottomSizer->AddStretchSpacer(1);

        sizer->Add(bottomSizer, 0, wxEXPAND);

        SetSizerAndFit(sizer);

        colorButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent &event) {
            wxColourData colorData;
            colorData.SetColour(this->canvas->triangleColor);
            wxColourDialog dialog(this, &colorData);
            if (dialog.ShowModal() == wxID_OK) {
                this->canvas->triangleColor = dialog.GetColourData().GetColour();
                this->canvas->Refresh();
            }
        }); // end of lambda

    }
};

// Your app, containing your frame
class MyApp : public wxApp
{
public:
    MyApp() {}
    bool OnInit() wxOVERRIDE;
};

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit()
{
    if (!wxApp::OnInit()) { return false; }
    MyFrame *frame = new MyFrame("Hello OpenGL");
    frame->Show(true);
    return true;
}
