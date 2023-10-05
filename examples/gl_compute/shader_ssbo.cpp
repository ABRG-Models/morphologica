#include <morph/gl_compute.h>
#include <morph/vvec.h>
#include <morph/loadpng.h>

/*
 * A second example of extending morph::gl_compute, this time to a shader which computes
 * using SSBOs - shader storage buffer objects.
 */

namespace my {

    struct gl_compute : public morph::gl_compute<3,1> // Use OpenGL 3.1 ES
    {
        static constexpr int dwidth = 256;
        static constexpr int dheight = 65;
        static constexpr int dsz = dwidth * dheight;

        // Call init in your constructor, ensuring *your* version of load_shaders() is called.
        gl_compute()
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
            glUseProgram (this->compute_program);
            glGenTextures (1, &this->texture1);
            //glActiveTexture (GL_TEXTURE0); // Doesn't appear to be necessary to call glActiveTexture() here.
            glBindTexture (GL_TEXTURE_2D, this->texture1);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA32F, tex_width, tex_height, 0, GL_RGBA, GL_FLOAT, NULL);
            //        args: ( unit, texture_id,level, layered,layer, access,     pixel_format)
            glBindImageTexture (0, this->texture1, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

            // Set up a second texture
            glGenTextures (1, &this->texture2);
            glBindTexture (GL_TEXTURE_2D, this->texture2);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA32F, tex_width, tex_height, 0, GL_RGBA, GL_FLOAT, NULL);
            //        args: ( unit, texture_id, level, layered,layer, access,     pixel_format)
            glBindImageTexture (1, this->texture2, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

            std::cout << "texture1: " << texture1 << ", texture2: " << texture2 << std::endl;

            // SSBO setup. First a local memory version of the input
            this->input.resize (dsz, 0.0f);
            morph::vec<unsigned int, 2> dims = morph::loadpng ("../examples/gl_compute/bike.png", this->input);
            std::cout << "Image dims: " << dims << std::endl;

            glUseProgram (this->compute_program);
            glGenBuffers (1, &this->input_buffer);
            glBindBufferBase (GL_SHADER_STORAGE_BUFFER, 1, this->input_buffer);
            glBufferData (GL_SHADER_STORAGE_BUFFER, input.size() * sizeof(float), input.data(), GL_STATIC_DRAW);
            glBindBuffer (GL_SHADER_STORAGE_BUFFER, 0);
        }

        ~gl_compute()
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
                {GL_COMPUTE_SHADER, "../examples/gl_compute/shader_ssbo.glsl", morph::defaultComputeShader }
            };
            this->compute_program = morph::gl::LoadShaders (shaders);

            // We'll reuse the vertex/fragment shaders from the shadercompute example
            std::vector<morph::gl::ShaderInfo> vtxshaders = {
                {GL_VERTEX_SHADER, "../examples/gl_compute/shader_ssbo.vert.glsl", morph::defaultVtxShader },
                {GL_FRAGMENT_SHADER, "../examples/gl_compute/shader_ssbo.frag.glsl", morph::defaultFragShader }
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
            // Fixme - make a program class that wraps this up.
            GLint uloc = glGetUniformLocation (this->compute_program, static_cast<const GLchar*>("t"));
            if (uloc != -1) { glUniform1f (uloc, this->frame_count); }

            // This is dispatch with work groups of (a, b, 1)
            glDispatchCompute (dwidth, dheight, 1);
            // make sure writing to image has finished before read
            glMemoryBarrier (GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
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

            glActiveTexture (GL_TEXTURE0); // Must also be GL_TEXTURE0, as the shader will act only on one texture
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
        // CPU side input data
        morph::vvec<float> input;
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
