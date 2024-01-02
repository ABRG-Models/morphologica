#if 0
// This isthe MyGLCanvas from the example
class MyGLCanvas: public wxGLCanvas
{
public:
    // We create a wxGLContext in this constructor.
    // We do OGL initialization at OnSize().
    MyGLCanvas (MyFrame* parent, const wxGLAttributes& canvasAttrs)
    {
        m_parent = parent;

        m_oglManager = nullptr;
        m_winHeight = 0; // We have not been sized yet

        // Explicitly create a new rendering context instance for this canvas.
        wxGLContextAttrs ctxAttrs;
#ifndef __WXMAC__
        // An impossible context, just to test IsOk()
        ctxAttrs.PlatformDefaults().OGLVersion(99, 2).EndList();
        m_oglContext = new wxGLContext(this, nullptr, &ctxAttrs);

        if ( !m_oglContext->IsOK() )
        {
#if wxUSE_LOGWINDOW
            wxLogMessage("Trying to set OpenGL 99.2 failed, as expected.");
#endif // wxUSE_LOGWINDOW
            delete m_oglContext;
            ctxAttrs.Reset();
#endif //__WXMAC__
            ctxAttrs.PlatformDefaults().CoreProfile().OGLVersion(3, 2).EndList();
            m_oglContext = new wxGLContext(this, nullptr, &ctxAttrs);
#ifndef __WXMAC__
        }
#endif //__WXMAC__

        if (!m_oglContext->IsOK()) {
            wxMessageBox("This sample needs an OpenGL 3.2 capable driver.\nThe app will end now.",
                         "OpenGL version error", wxOK | wxICON_INFORMATION, this);
            delete m_oglContext;
            m_oglContext = nullptr;
        }
        else
        {
#if wxUSE_LOGWINDOW
            wxLogMessage("OpenGL Core Profile 3.2 successfully set.");
#endif // wxUSE_LOGWINDOW
        }
    }

    ~MyGLCanvas()
    {
        if (m_oglContext) { SetCurrent(*m_oglContext); }

        if (m_oglManager) {
            delete m_oglManager;
            m_oglManager = nullptr;
        }

        if (m_oglContext) {
            delete m_oglContext;
            m_oglContext = nullptr;
        }
    }

    // Used just to know if we must end now because OGL isn't available
    bool OglCtxAvailable()
    {
        return this->m_oglContext != nullptr;
    }

    // Init the OpenGL stuff
    bool oglInit()
    {
        if (!m_oglContext) { return false; }

        // The current context must be set before we get OGL pointers
        SetCurrent(*m_oglContext);

        // Initialize our OGL pointers
        if (!myOGLManager::Init()) {
            wxMessageBox("Error: Some OpenGL pointer to function failed.",
                         "OpenGL initialization error", wxOK | wxICON_INFORMATION, this);
            return false;
        }

        // Create our OGL manager, pass our OGL error handler
        m_oglManager = new myOGLManager(&fOGLErrHandler);

        // Get the GL version for the current OGL context
        wxString sglVer = "\nUsing OpenGL version: ";
        sglVer += wxString::FromUTF8(
            reinterpret_cast<const char *>(m_oglManager->GetGLVersion()) );
        // Also Vendor and Renderer
        sglVer += "\nVendor: ";
        sglVer += wxString::FromUTF8(
            reinterpret_cast<const char *>(m_oglManager->GetGLVendor()) );
        sglVer += "\nRenderer: ";
        sglVer += wxString::FromUTF8(
            reinterpret_cast<const char *>(m_oglManager->GetGLRenderer()) );
        // For the menu "About" info
        m_parent->SetOGLString(sglVer);

        // Load some data into GPU
        m_oglManager->SetShadersAndTriangles();

        // This string will be placed on a face of the pyramid
        int swi = 0, shi = 0; //Image sizes
        wxString stg("wxWidgets");
        // Set the font. Use a big pointsize so as to smoothing edges.
        wxFont font(wxFontInfo(48).Family(wxFONTFAMILY_MODERN));
        if ( !font.IsOk() )
            font = *wxSWISS_FONT;
        wxColour bgrdColo(*wxBLACK);
        wxColour foreColo(160, 0, 200); // Dark purple
        // Build an array with the pixels. Background fully transparent
        unsigned char* sPixels = MyTextToPixels(stg, font, foreColo, bgrdColo, 0,
                                                &swi, &shi);
        // Send it to GPU
        m_oglManager->SetStringOnPyr(sPixels, swi, shi);
        delete[] sPixels; // That memory was allocated at MyTextToPixels

        // This string is placed at left bottom of the window. Its size doesn't
        // change with window size.
        stg = "Rotate the pyramid with\nthe left mouse button";
        font.SetPointSize(14);
        bgrdColo = wxColour(40, 40, 255);
        foreColo = wxColour(*wxWHITE);
        unsigned char* stPixels = MyTextToPixels(stg, font, foreColo, bgrdColo, 80,
                                                 &swi, &shi);
        m_oglManager->SetImmutableString(stPixels, swi, shi);
        delete[] stPixels;

        return true;
    }

    void OnPaint (wxPaintEvent& event)
    {
        // This is a dummy, to avoid an endless succession of paint messages.
        // OnPaint handlers must always create a wxPaintDC.
        wxPaintDC dc(this);

        // Avoid painting when we have not yet a size
        if (m_winHeight < 1 || !m_oglManager) { return; }

        // This should not be needed, while we have only one canvas
        SetCurrent(*m_oglContext);

        // Do the magic
        m_oglManager->Render();

        SwapBuffers();
    }

    // Note:
    // You may wonder why OpenGL initialization was not done at wxGLCanvas ctor.
    // The reason is due to GTK+/X11 working asynchronously, we can't call
    // SetCurrent() before the window is shown on screen (GTK+ doc's say that the
    // window must be realized first).
    // In wxGTK, window creation and sizing requires several size-events. At least
    // one of them happens after GTK+ has notified the realization. We use this
    // circumstance and do initialization then.
    void OnSize (wxSizeEvent& event)
    {
        event.Skip();

        // If this window is not fully initialized, dismiss this event
        if ( !IsShownOnScreen() )
            return;

        if ( !m_oglManager )
        {
            //Now we have a context, retrieve pointers to OGL functions
            if ( !oglInit() )
                return;
            //Some GPUs need an additional forced paint event
            PostSizeEvent();
        }

        // This is normally only necessary if there is more than one wxGLCanvas
        // or more than one wxGLContext in the application.
        SetCurrent(*m_oglContext);

        // It's up to the application code to update the OpenGL viewport settings.
        const wxSize size = event.GetSize() * GetContentScaleFactor();
        m_winHeight = size.y;
        m_oglManager->SetViewport(0, 0, size.x, m_winHeight);

        // Generate paint event without erasing the background.
        Refresh(false);
    }

    void OnMouse (wxMouseEvent& event)
    {
        event.Skip();

        // GL 0 Y-coordinate is at bottom of the window
        int oglwinY = m_winHeight - event.GetY();

        if (event.LeftIsDown()) {
            if (!event.Dragging()) {
                // Store positions
                m_oglManager->OnMouseButDown (event.GetX(), oglwinY);
            } else {
                // Rotation
                m_oglManager->OnMouseRotDragging (event.GetX(), oglwinY);

                // Generate paint event without erasing the background.
                Refresh (false);
            }
        }
    }


private:
    // Members
    MyFrame*      m_parent = nullptr;
    wxGLContext*  m_oglContext = nullptr;
    myOGLManager* m_oglManager = nullptr;
    int           m_winHeight = -1;       // We use this var to know if we have been sized

    wxDECLARE_EVENT_TABLE();
};
#endif
