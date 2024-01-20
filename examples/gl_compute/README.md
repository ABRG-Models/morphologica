# OpenGL compute shader examples

This directory contains a few examples of how to use the compute
shader classes in morph/gl/.

## shadercompute.cpp

This is an implementation of an OpenGL shader compute article from [learnopengl.com](https://learnopengl.com/Guest-Articles/2022/Compute-Shaders/Introduction).

It introduces a 'compute manager' class `morph::gl::compute_manager`.

The code is in **examples/gl_compute/shadercompute.cpp**. It has three shaders; a compute shader **shadercompute.glsl** and two graphics shaders: **shadercompute.vert.glsl** and **shadercompute.frag.glsl**.

It computes a red, green and yellow texture which is repeatedly shifted, so that it slides to the left.

## shader_ssbo.cpp

This is another GL compute example. This one demonstrates the use of shader storage buffer objects with the `morph::gl::ssbo` class.

It copies a photo of a bicycle which is transferred into textures and displayed.

## naive_scan.cpp

This is **not** a gl shader program, but is instead a CPU implementation of the prefix sum algorithm, which I used to test the shader implementations of the same summing algorithm.

The input is the simple sequence [0,1,2,3,4,...,29,30,31] in `float` format.

The output is the running sum of the sequence: [0,1,3,6,10,...,435,465,496].

Builds into the executable **seq_naive_scan**.

## shader_naive_scan.cpp

Compute prefix sum in a shader. This is a very common algorithm in compute shaders. The algorithm is found in **naive_scan.glsl**. Output is shown on stdout.

## shader_naive_scan_cli.cpp

Same as shader_naive_scan.cpp but uses the `morph::gl::compute_manager_cli` base class, which allows you to do GL compute shader operations on your GPU without a display. Uses EGL (you need libgbm, too).
