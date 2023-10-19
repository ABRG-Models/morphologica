/*
 * How to make a compute shader with morph::gl::compute_manager
 *
 * 1) Extend morph::compute_manager to add the data structures and compute_programs that you will
 * need for your computation
 * 2) Write the compute glsl files
 * 3) Create an object of your morph::gl::compute_manager class, call init() and set its compute inputs
 * 4) call the compute() method
 * 5) Read the results from your compute_manager class's output attributes
 *
 * This example was constructed by following and adapting the tutorial at:
 * https://learnopengl.com/Guest-Articles/2022/Compute-Shaders/Introduction
 */


// You have to include the GL headers manually so that you will be sure you have the
// right ones. THis is because for OpenGL version 4.3, you would include GL3/gl3.h and
// GL/glext.h whereas if you are targeting OpenGL 3.1 ES, you want to include
// GLES3/gl3[12].h (and maybe GLES3/gl3ext.h). These includes are required before
// including gl_compute.h.
#include <GL3/gl3.h>
#include <GL/glext.h>

#include <morph/gl/compute_manager.h>
#include <morph/gl/texture.h>

namespace my {

    // Specify OpenGL version 4.5 (4.3 min for compute)
    static constexpr int gl_version_major = 4;
    static constexpr int gl_version_minor = 5;
    static constexpr bool gles = false;

    struct compute_manager : public morph::gl::compute_manager<gl_version_major, gl_version_minor, gles>
    {
        // Call init in your constructor, ensuring *your* version of load_shaders() is called.
        compute_manager()
        {
            this->init();
            // Set up buffers for visualisation
            float quadVertices[] = {
                // positions        // texture Coords
                -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
                 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
            };
            // setup plane VAO
            glGenVertexArrays (1, &this->vao);
            glGenBuffers (1, &this->vbo);
            glBindVertexArray (this->vao);
            glBindBuffer (GL_ARRAY_BUFFER, this->vbo);
            glBufferData (GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

            // Texture setup
            this->compute_program.use();
            GLuint itu = 0; // Image texture unit
            morph::vec<GLsizei, 2> dims = { tex_width, tex_height };
            morph::gl::setup_texture (itu, this->texture, dims);
        }
        ~compute_manager()
        {
            // Clean up buffers
            if (this->vao > 0) {
                glDeleteBuffers (1, &this->vbo);
                glDeleteVertexArrays (1, &this->vao);
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
                {GL_COMPUTE_SHADER, "../examples/gl_compute/shadercompute.glsl", morph::gl::nonCompilingComputeShader }
            };
            this->compute_program.load_shaders (shaders);

            std::vector<morph::gl::ShaderInfo> vtxshaders = {
                {GL_VERTEX_SHADER, "../examples/gl_compute/shadercompute.vert.glsl", morph::defaultVtxShader },
                {GL_FRAGMENT_SHADER, "../examples/gl_compute/shadercompute.frag.glsl", morph::defaultFragShader }
            };
            this->vtxprog = morph::gl::LoadShaders (vtxshaders);
        }

        // Override your one time/non-rendering compute function
        void compute() final
        {
            this->measure_compute(); // optional
            this->compute_program.use();
            // Set time into a uniform in the compute program
            this->compute_program.set_uniform<float> ("t", this->frame_count);
            // This is dispatch with work groups of (a, b, 1)
            this->compute_program.dispatch (tex_width/10, tex_height/10, 1);
        }

        // Override the render method to do whatever visualization you want
        void render() final
        {
            // Compute again on each render for this example
            this->compute();

            // render image to quad
            glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glUseProgram (this->vtxprog);

            // Set a uniform variable called "tex" in the shader program to 0. This is
            // the texture sampler in the fragment shader.
            glUniform1i (glGetUniformLocation(this->vtxprog, "tex"), 0);

            glActiveTexture (GL_TEXTURE0);
            glBindTexture (GL_TEXTURE_2D, this->texture);

            // Bind vertex array and draw the triangles
            glBindVertexArray (this->vao);
            glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
            glBindVertexArray(0);

            glfwSwapBuffers (this->window);
            glfwPollEvents();
        }

    private:
        // Add any members required in your compute class
        static constexpr unsigned int tex_width = 1000;
        static constexpr unsigned int tex_height = 1000;
        // A texture ID
        unsigned int texture = 0;
        // A vertex shader program
        GLuint vtxprog = 0;
        // Vertex array/buffer objects used for visualization in render()
        unsigned int vao = 0;
        unsigned int vbo = 0;
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
