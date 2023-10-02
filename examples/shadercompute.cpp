#include <morph/gl_compute.h>

/*
 * How to make a compute shader with morphologica.
 *
 * 1) Extend morph::gl_compute to add the data structures that you will need for your computation.
 * 2) Write a compute glsl file
 * 3) Create an object of your gl_compute class and set its compute inputs
 * 4) call the compute() method
 * 5) Read the results from your gl_compute class's output attributes
 */

int main()
{
    morph::gl_compute gc;
    return 0;
}
