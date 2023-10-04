#include <morph/gl_compute.h>
#include <morph/vvec.h>

/*
 * A second example of extending morph::gl_compute, this time to a shader which computes
 * using SSBOs - shader storage buffer objects
 */

namespace my {

    struct gl_compute : public morph::gl_compute
    {
        static constexpr int dwidth = 256;
        static constexpr int dheight = 65;
        static constexpr int dsz = dwidth * dheight;

        // Call init in your constructor, ensuring *your* version of load_shaders() is called.
        gl_compute()
        {
            // Your GLFW window will take the size in win_sz.
            this->win_sz = {dwidth, dheight};

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

            // SSBO setup. First a local memory version of the input
            morph::vvec<float> input (dsz, 0.0f);
            glGenBuffers (1, &this->input_buffer);
            glBindBufferBase (GL_SHADER_STORAGE_BUFFER, 1, this->input_buffer);
            glBufferData (GL_SHADER_STORAGE_BUFFER, sizeof(input.data()), input.data(), GL_STATIC_DRAW);
            glBindBuffer (GL_SHADER_STORAGE_BUFFER, 0);
        }
        ~gl_compute()
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
                {GL_COMPUTE_SHADER, "../examples/shaderuniform.glsl", morph::defaultComputeShader }
            };
            this->compute_program = morph::gl::LoadShaders (shaders);

            // We'll reuse the vertex/fragment shaders from the shadercompute example
            std::vector<morph::gl::ShaderInfo> vtxshaders = {
                {GL_VERTEX_SHADER, "../examples/shadercompute.vert.glsl", morph::defaultVtxShader },
                {GL_FRAGMENT_SHADER, "../examples/shadercompute.frag.glsl", morph::defaultFragShader }
            };
            this->vtxprog = morph::gl::LoadShaders (vtxshaders);
        }

        // Override your one time/non-rendering compute function
        void compute() final
        {
            this->measure_compute(); // optional

            glUseProgram (this->compute_program);

            // Set time into uniform
            // This is nice, you can access a uniform variable in the GLSL using its variable name ("t")
            GLint uloc = glGetUniformLocation (this->compute_program, static_cast<const GLchar*>("t"));
            if (uloc != -1) { glUniform1f (uloc, this->frame_count); }

            // This is dispatch with work groups of (a, b, 1)
            glDispatchCompute (tex_width/10, tex_height/10, 1);
            // make sure writing to image has finished before read
            glMemoryBarrier (GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        }

        // Override the render method to do whatever visualization you want
        void render() final
        {
            // Compute again on each render for this example
            this->compute();

            // render image to quad
            glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glUseProgram (this->vtxprog);

            // Activate the texture and get it drawn
            glActiveTexture (GL_TEXTURE0);
            glBindTexture (GL_TEXTURE_2D, this->texture);
            glBindVertexArray (this->vao);
            glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
            glBindVertexArray(0);

            // Swap buffers and poll for events
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

        // ID for the input SSBO buffer
        unsigned int input_buffer = 0;
    };
} // namespace my

int main()
{
    my::gl_compute c;
    while (!c.readyToFinish) { c.render(); } // Render also calls compute

    // You could compute very fast without render (I got 1.6 mega-fps) but this may
    // interfere with your desktop's responsiveness
    // while (!c.readyToFinish) { c.compute(); }

    return 0;
}
