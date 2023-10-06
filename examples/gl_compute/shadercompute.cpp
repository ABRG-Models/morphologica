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

    struct gl_compute : public morph::gl_compute<4,5> // Specify OpenGL version 4.5 (4.3 min for compute)
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

            // Texture setup
            this->compute_program.use();
            GLuint itu = 0; // Image texture unit
            glGenTextures (1, &this->texture);
            //glActiveTexture (GL_TEXTURE0+itu); // Sets the active texture to GL_TEXTURE0+itu where itu is Image Texture Unit. But this is not necessary as we bind image texture below with ITU=0
            glBindTexture (GL_TEXTURE_2D, this->texture);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA32F, tex_width, tex_height, 0, GL_RGBA, GL_FLOAT, NULL);
            std::cout << "GL_TEXTURE0: " << GL_TEXTURE0 << std::endl;
            glBindImageTexture (itu, this->texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
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
                {GL_COMPUTE_SHADER, "../examples/gl_compute/shadercompute.glsl", morph::defaultComputeShader }
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
