/*
 * To become a wx-canvas-with-morph::Visual
 *
 * Defined morph::wx::Canvas and morph::wx::Frame.
 */
#include <memory>
#include <wx/wx.h>

#include <GL/glew.h> // must be included before glcanvas.h
#include <wx/glcanvas.h>
#include <wx/colordlg.h>

namespace morph {
    namespace wx {

        class Canvas : public wxGLCanvas
        {
        public:
            Canvas (wxFrame* parent, const wxGLAttributes& canvasAttrs)
                : wxGLCanvas(parent, canvasAttrs)
            {
                wxGLContextAttrs ctxAttrs;
                ctxAttrs.PlatformDefaults().CoreProfile().OGLVersion(4, 1).EndList();
                this->glContext = std::make_unique<wxGLContext>(this, nullptr, &ctxAttrs);

                if (!this->glContext->IsOK()) {
                    wxMessageBox ("This sample needs an OpenGL 4.1 capable driver.",
                                  "OpenGL version error", wxOK | wxICON_INFORMATION, this);
                }

                Bind(wxEVT_PAINT, &morph::wx::Canvas::OnPaint, this);
                Bind(wxEVT_SIZE, &morph::wx::Canvas::OnSize, this);
            }

            bool InitializeOpenGLFunctions()
            {
                GLenum err = glewInit();
                if (GLEW_OK != err) {
                    wxLogError("OpenGL GLEW initialization failed: %s",
                               reinterpret_cast<const char *>(glewGetErrorString(err)));
                    return false;
                }
                wxLogDebug("Status: Using GLEW %s", reinterpret_cast<const char *>(glewGetString(GLEW_VERSION)));
                return true;
            }

            // *this* will change with morph::Visual
            bool InitializeOpenGL()
            {
                if (!this->glContext) { return false; }

                SetCurrent (*this->glContext.get());

                if (!InitializeOpenGLFunctions()) {
                    wxMessageBox("Error: Could not initialize OpenGL function pointers.",
                                 "OpenGL initialization error", wxOK | wxICON_INFORMATION, this);
                    return false;
                }

                wxLogDebug("OpenGL version: %s", reinterpret_cast<const char *>(glGetString(GL_VERSION)));
                wxLogDebug("OpenGL vendor: %s", reinterpret_cast<const char *>(glGetString(GL_VENDOR)));

                constexpr auto vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        void main()
        {
            gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
        }
    )";

                constexpr auto fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        uniform vec4 triangleColor;
        void main()
        {
            FragColor = triangleColor;
        }
    )";

                unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
                glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
                glCompileShader(vertexShader);

                int success;
                char infoLog[512];
                glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

                if (!success) {
                    glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
                    wxLogDebug("Vertex Shader Compilation Failed: %s", infoLog);
                }

                unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
                glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
                glCompileShader(fragmentShader);

                glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

                if (!success) {
                    glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
                    wxLogDebug("Fragment Shader Compilation Failed: %s", infoLog);
                }

                shaderProgram = glCreateProgram();
                glAttachShader(shaderProgram, vertexShader);
                glAttachShader(shaderProgram, fragmentShader);
                glLinkProgram(shaderProgram);

                glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

                if (!success) {
                    glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
                    wxLogDebug("Shader Program Linking Failed: %s", infoLog);
                }

                glDeleteShader(vertexShader);
                glDeleteShader(fragmentShader);

                float vertices[] = {
                    -0.5f, -0.5f, 0.0f,
                    0.5f, -0.5f, 0.0f,
                    0.0f, 0.5f, 0.0f};

                glGenVertexArrays(1, &VAO);
                glGenBuffers(1, &VBO);

                glBindVertexArray(VAO);
                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
                glEnableVertexAttribArray(0);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindVertexArray(0);

                this->glInitialized = true;

                return true;
            }


            void OnPaint (wxPaintEvent &event)
            {
                wxPaintDC dc(this);
                if (!this->glInitialized) { return; }
                SetCurrent(*this->glContext.get());
                glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);
                glUseProgram(shaderProgram);
                int colorLocation = glGetUniformLocation(shaderProgram, "triangleColor");
                glUniform4f(colorLocation,
                            triangleColor.Red() / 255.0f, triangleColor.Green() / 255.0f, triangleColor.Blue() / 255.0f, 1.0f);
                glBindVertexArray(VAO);
                glDrawArrays(GL_TRIANGLES, 0, 3);
                SwapBuffers();
            }

            void OnSize (wxSizeEvent &event)
            {
                bool firstApperance = IsShownOnScreen() && !this->glInitialized;
                if (firstApperance) { InitializeOpenGL(); }
                if (this->glInitialized) {
                    auto viewPortSize = event.GetSize() * GetContentScaleFactor();
                    glViewport(0, 0, viewPortSize.x, viewPortSize.y);
                }
                event.Skip();
            }

            wxColour triangleColor{wxColour(255, 128, 51)};

        private:
            std::unique_ptr<wxGLContext> glContext;
            bool glInitialized{false};

            unsigned int VAO, VBO, shaderProgram;
        };

        // morph::wx::Frame is to be extended
        class Frame : public wxFrame
        {
        public:
            Frame(const wxString &title)
                : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxDefaultSize)
            {
                wxGLAttributes vAttrs;
                vAttrs.PlatformDefaults().Defaults().EndList();
                if (wxGLCanvas::IsDisplaySupported(vAttrs)) {
                    this->canvas = std::make_unique<morph::wx::Canvas>(this, vAttrs);
                    this->canvas->SetMinSize (FromDIP (wxSize(640, 480)));
                } else {
                    throw std::runtime_error ("wxGLCanvas::IsDisplaySupported(vAttrs) returned false");
                }
            }

        protected:
            std::unique_ptr<morph::wx::Canvas> canvas;
        };

    } // wx
} // morph
