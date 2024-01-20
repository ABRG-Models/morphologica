/*
 * Display-free GL compute shader example.
 *
 * A shader example implementing naive parallel prefix scan (using SSBOs - shader storage buffer
 * objects to transfer data to and from the GPU)
 *
 * This differs from shader_naive_scan.cpp in that it derives its compute_manager from
 * morph::gl::compute_manager_cli, which allows you to perform compute shader
 * computations without any display at all. It uses EGL to achieve this.
 */

// You have to include the GL headers manually so that you will be sure you have the
// right ones. THis is because for OpenGL version 4.3, you would include GL3/gl3.h and
// GL/glext.h whereas if you are targeting OpenGL 3.1 ES, you want to include
// GLES3/gl3[12].h (and maybe GLES3/gl3ext.h).
#include <GLES3/gl31.h>

#include <morph/gl/compute_manager_cli.h>
#include <morph/gl/texture.h>
#include <morph/gl/ssbo.h>
#include <morph/vvec.h>
#include <morph/loadpng.h>

namespace my {

    // Use OpenGL 3.1 ES here
    struct compute_manager : public morph::gl::compute_manager_cli<morph::gl::version_3_1_es>
    {
        static constexpr int dsz = 32;

        // Call init in your constructor, ensuring *your* version of load_shaders() is called.
        compute_manager()
        {
            this->init();

            // Set that data into the SSBO object (where it is stored in a vec<>)
            float n = 0.0f;
            for (auto& f : this->input_ssbo.data) {
                f = n;
                n += 1.0f;
            }
            this->input_ssbo.init();
            this->output_ssbo.data.zero();
            this->output_ssbo.init();
            this->debug_ssbo.data.zero();
            this->debug_ssbo.init();
            this->debug2_ssbo.data.zero();
            this->debug2_ssbo.init();
        }

        ~compute_manager(){}

        // Override load_shaders() to load whatever shaders you need.
        void load_shaders() final
        {
            std::vector<morph::gl::ShaderInfo> shaders = {
                {GL_COMPUTE_SHADER, "../examples/gl_compute/naive_scan.glsl", morph::gl::nonCompilingComputeShader, 0 }
            };
            this->scan_program.load_shaders (shaders);
        }

        double compstep = 0.0;
        // Override your one time/non-rendering compute function
        void compute() final
        {
            this->input_ssbo.copy_to_gpu(); // should have happened in init?

            this->scan_program.use();
            this->scan_program.dispatch (dsz, 1, 1);

            this->output_ssbo.copy_from_gpu();
            this->debug_ssbo.copy_from_gpu();
            this->debug2_ssbo.copy_from_gpu();
            std::cout << "Prefix sum input:\n" << this->input_ssbo.data << std::endl;
            std::cout << "\nDebug data1:\n" << this->debug_ssbo.data << std::endl;
            std::cout << "Debug data2:\n" << this->debug2_ssbo.data << std::endl;
            std::cout << "\nPrefix sum result:\n" << this->output_ssbo.data << std::endl;
        }

    private:
        // Add any members required in your compute class
        // CPU side input data. This will be SSBO index 1.
        morph::gl::ssbo<1, float, dsz> input_ssbo;
        morph::gl::ssbo<2, float, dsz> output_ssbo;
        morph::gl::ssbo<3, float, dsz> debug_ssbo;
        morph::gl::ssbo<4, float, dsz> debug2_ssbo;
        // You will need at least one gl::compute_shaderprog
        morph::gl::compute_shaderprog<morph::gl::version_3_1_es> scan_program;
    };
} // namespace my

int main()
{
    my::compute_manager c;
    c.compute();
    return 0;
}
