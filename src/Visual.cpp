#include "Visual.h"

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

void
morph::Visual::errorCallback (int error, const char* description)
{
    cerr << "Error: " << description << " (code "  << error << ")" << endl;
}

morph::Visual::Visual(int width, int height, const string& title)
{
    if (!glfwInit()) {
        // Initialization failed
        cerr << "GLFW initialization failed!" << endl;
    }

    glfwSetErrorCallback (morph::Visual::errorCallback);


    glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 5);
    this->window = glfwCreateWindow (width, height, title.c_str(), NULL, NULL);
    if (!this->window) {
        // Window or OpenGL context creation failed
        cerr << "GLFW window creation failed!" << endl;
    }
}

morph::Visual::~Visual()
{
    glfwDestroyWindow (this->window);
    glfwTerminate();
}

void
morph::Visual::redraw (void)
{
}

void
morph::Visual::updateHexGridVisual (const unsigned int gridId,
                                    const vector<float>& data)
{
}

unsigned int
morph::Visual::addHexGridVisual (const HexGrid* hg,
                                 const vector<float>& data,
                                 const array<float, 3> offset)
{
    return 0;
}
