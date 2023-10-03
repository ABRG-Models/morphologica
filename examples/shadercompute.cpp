#include <morph/gl_compute.h>

/*
 * How to make a compute shader with morph::gl_compute
 *
 * 1) Extend morph::gl_compute to add the data structures that you will need for your computation.
 * 2) Write a compute glsl file
 * 3) Create an object of your gl_compute class, call init() and set its compute inputs
 * 4) call the compute() method
 * 5) Read the results from your gl_compute class's output attributes
 *
 * This example was constructed by following and adapting the tutorial at:
 * https://learnopengl.com/Guest-Articles/2022/Compute-Shaders/Introduction
 */

namespace my {

    struct gl_compute : public morph::gl_compute
    {
        // Call init in your constructor, ensuring *your* version of load_shaders() is called.
        gl_compute()
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
                {GL_COMPUTE_SHADER, "../examples/shadercompute.glsl", morph::defaultComputeShader }
            };
            this->compute_program = morph::gl::LoadShaders (shaders);

            std::vector<morph::gl::ShaderInfo> vtxshaders = {
                {GL_VERTEX_SHADER, "../examples/shadercompute.vert.glsl", morph::defaultVtxShader },
                {GL_FRAGMENT_SHADER, "../examples/shadercompute.frag.glsl", morph::defaultFragShader }
            };
            this->vtxprog = morph::gl::LoadShaders (vtxshaders);
        }

        // Override your one time/non-rendering compute function
        void compute() final
        {
            glUseProgram (this->compute_program);

            glGenTextures (1, &this->texture);
            glActiveTexture (GL_TEXTURE0);

            glBindTexture (GL_TEXTURE_2D, this->texture);

            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA32F, tex_width, tex_height, 0, GL_RGBA, GL_FLOAT, NULL);

            glBindImageTexture (0, this->texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

            // This is dispatch with work groups of (512, 512, 1)
            glDispatchCompute (tex_width, tex_height, 1);
            // make sure writing to image has finished before read
            glMemoryBarrier (GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        }

        // Override the render method to do whatever visualization you want
        void render() final
        {
            // Compute again on each render for this example
            glUseProgram (this->compute_program);
            glDispatchCompute (tex_width, tex_height, 1);
            glMemoryBarrier (GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            // render image to quad
            glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glUseProgram (this->vtxprog);
            // Set the thing called "tex" to 0
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
        static constexpr unsigned int tex_width = 512;
        static constexpr unsigned int tex_height = 512;
        // A texture ID
        unsigned int texture = 0;
        // A vertex shader program
        GLuint vtxprog = 0;
        // Vertex array/buffer objects used for visualization in render()
        unsigned int vao = 0;
        unsigned int vbo = 0;
    };
} // namespace my

int main()
{
    my::gl_compute c;
    c.compute();
    while (!c.readyToFinish) { c.render(); }
    return 0;
}
