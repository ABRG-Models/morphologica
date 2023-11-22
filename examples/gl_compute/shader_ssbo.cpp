/*
 * A second example of extending morph::gl_compute, this time to a shader which computes
 * using SSBOs - shader storage buffer objects.
 */

// You have to include the GL headers manually so that you will be sure you have the
// right ones. THis is because for OpenGL version 4.3, you would include GL3/gl3.h and
// GL/glext.h whereas if you are targeting OpenGL 3.1 ES, you want to include
// GLES3/gl3[12].h (and maybe GLES3/gl3ext.h).
#include <GLES3/gl31.h>

#include <morph/gl/compute_manager.h>
#include <morph/gl/texture.h>
#include <morph/gl/ssbo.h>
#include <morph/vvec.h>
#include <morph/loadpng.h>

namespace my {

    // Use OpenGL 3.1 ES here
    static constexpr int gl_version_major = 3;
    static constexpr int gl_version_minor = 1;
    static constexpr bool gles = true;

    struct compute_manager : public morph::gl::compute_manager<gl_version_major, gl_version_minor, gles>
    {
        static constexpr int dwidth = 256;
        static constexpr int dheight = 65;
        static constexpr int dsz = dwidth * dheight;

        // Call init in your constructor, ensuring *your* version of load_shaders() is called.
        compute_manager()
        {
            // Your GLFW window will take the size in win_sz.
            this->win_sz = {dwidth*8, dheight*8};

            this->init();

            // setup plane vertex array object for rendering an output texture. This is used simply
            // to visually verify the operations carried out in the compute shader.
            float leftbox[] = {
                // positions        // texture Coords
                -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                 0.0f,  1.0f, 0.0f, 1.0f, 1.0f,
                 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
            };
            glGenVertexArrays (1, &this->vao1);
            glGenBuffers (1, &this->vbo1);
            glBindVertexArray (this->vao1);
            glBindBuffer (GL_ARRAY_BUFFER, this->vbo1);
            glBufferData (GL_ARRAY_BUFFER, sizeof(leftbox), &leftbox, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

            float rightbox[] = {
                // positions        // texture Coords
                0.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
            };
            glGenVertexArrays (1, &this->vao2);
            glGenBuffers (1, &this->vbo2);
            glBindVertexArray (this->vao2);
            glBindBuffer (GL_ARRAY_BUFFER, this->vbo2);
            glBufferData (GL_ARRAY_BUFFER, sizeof(rightbox), &rightbox, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

            // Set up the texture for output
            this->compute_program.use();
            GLuint itu = 0; // Image texture unit. The first compute shader texture will be itu 0 in the compute shader program.
            morph::vec<GLsizei, 2> dims = { tex_width, tex_height };

            // First texture
            morph::gl::setup_texture (itu, this->texture1, dims);
            // A second texture
            morph::gl::setup_texture (++itu, this->texture2, dims);

            std::cout << "texture1: " << texture1 << ", texture2: " << texture2 << std::endl;

            // SSBO setup. First load input into a morph::vvec
            morph::vvec<float> inputvv (dsz, 0.0f);
            morph::vec<unsigned int, 2> _dims = morph::loadpng ("../examples/gl_compute/bike.png", inputvv);
            if ((_dims - dims) > 0) { throw std::runtime_error ("Loaded image is not expected size"); }

            // Set that data into the SSBO object (where it is stored in a vec<>)
            std::copy (inputvv.begin(), inputvv.end(), this->input_ssbo.data.begin());
            this->input_ssbo.init();
        }

        ~compute_manager()
        {
            // Clean up buffers
            if (this->vao1 > 0) {
                glDeleteBuffers (1, &this->vbo1);
                glDeleteVertexArrays (1, &this->vao1);
            }
            if (this->vao2 > 0) {
                glDeleteBuffers (1, &this->vbo2);
                glDeleteVertexArrays (1, &this->vao2);
            }
            // Clean up vertex shader prog
            if (this->vtxprog) {
                glDeleteShader (this->vtxprog);
                this->vtxprog = 0;
            }
        }
        // Override load_shaders() to load whatever shaders you need.
        void load_shaders() final
        {
            std::vector<morph::gl::ShaderInfo> shaders = {
                // Here I set up to load examples/shadercompute.glsl and leave the default shader in
                // place (which I don't intend to use).
                {GL_COMPUTE_SHADER, "../examples/gl_compute/shader_ssbo.glsl", morph::gl::nonCompilingComputeShader, 0 }
            };
            this->compute_program.load_shaders (shaders);

            // We'll reuse the vertex/fragment shaders from the shadercompute example
            std::vector<morph::gl::ShaderInfo> vtxshaders = {
                {GL_VERTEX_SHADER, "../examples/gl_compute/shader_ssbo.vert.glsl", morph::defaultVtxShader, 0 },
                {GL_FRAGMENT_SHADER, "../examples/gl_compute/shader_ssbo.frag.glsl", morph::defaultFragShader, 0 }
            };
            this->vtxprog = morph::gl::LoadShaders (vtxshaders);
        }

        double compstep = 0.0;
        // Override your one time/non-rendering compute function
        void compute() final
        {
            // To copy updated data:
            this->input_ssbo.data[0] = std::abs (std::sin (compstep)); // Make a pixel pulse
            compstep += 0.0001;
            this->input_ssbo.copy_to_gpu();

            this->measure_compute(); // optional
            this->compute_program.use();
            // Set time into uniform
            this->compute_program.set_uniform<float> ("t", this->frame_count);
            this->compute_program.dispatch (dwidth, dheight, 1);

            // To retreive data from the SSBO:
            // morph::range<float> ssbo_range = this->input_ssbo.get_range();
            // or
            this->input_ssbo.copy_from_gpu();
            // and access input_ssbo.data.
        }

        // Override the render method to do whatever visualization you want
        void render() final
        {
            // Compute again on each render for this example
            this->compute();

            // Clear the screen
            glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glUseProgram (this->vtxprog);

            // Activate each texture and draw on its relvant vertex array object.
            glActiveTexture (GL_TEXTURE0);
            glBindTexture (GL_TEXTURE_2D, this->texture1);
            glBindVertexArray (this->vao1);
            glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);

            glActiveTexture (GL_TEXTURE0); // Must *also* be GL_TEXTURE0, as the vertex/frag shaders will act only on one texture at a tiome
            glBindTexture (GL_TEXTURE_2D, this->texture2);
            glBindVertexArray (this->vao2);
            glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);

            glBindVertexArray(0);

            // Swap buffers and poll for events
            glfwSwapBuffers (this->window);
            glfwPollEvents();

            morph::gl::Util::checkError (__FILE__, __LINE__);
        }

    private:
        // Add any members required in your compute class
        static constexpr unsigned int tex_width = dwidth;
        static constexpr unsigned int tex_height = dheight;
        // A texture ID
        unsigned int texture1 = 0;
        unsigned int texture2 = 0;
        // A vertex shader program
        GLuint vtxprog = 0;
        // Vertex array/buffer objects used for visualization in render()
        unsigned int vao1 = 0;
        unsigned int vbo1 = 0;
        unsigned int vao2 = 0;
        unsigned int vbo2 = 0;
        // CPU side input data. This will be SSBO index 1.
        morph::gl::ssbo<1, float, dsz> input_ssbo;
        // You will need at least one gl::compute_shaderprog
        morph::gl::compute_shaderprog<gl_version_major, gl_version_minor, gles> compute_program;
    };
} // namespace my

int main()
{
    my::compute_manager c;
    while (!c.readyToFinish) { c.render(); } // Render also calls compute

    // You could compute very fast without render (I got 1.6 mega-fps) but this may
    // interfere with your desktop's responsiveness
    // while (!c.readyToFinish) { c.compute(); }

    return 0;
}
