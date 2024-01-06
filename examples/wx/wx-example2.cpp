#include <wx/wx.h>

#include <GL/glew.h> // must be included before glcanvas.h
#include <wx/glcanvas.h>

#include <wx/colordlg.h>

#include <morph/wx/viswx.h>

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

    morph::wx::Frame *frame = new morph::wx::Frame("Hello OpenGL");
    frame->Show(true);

    return true;
}
