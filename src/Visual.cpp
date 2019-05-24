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
morph::Visual::render (void)
{
    // Render it.
    vector<HexGridVisual*>::iterator hgvi = this->hexGridVis.begin();
    while (hgvi != this->layers.end()) {
        (*hgvi)->render();
        ++hgvi;
    }
}

void
morph::Visual::updateHexGridVisual (const unsigned int gridId,
                                    const vector<float>& data)
{
    // Replace grids[gridId].data
}

unsigned int
morph::Visual::addHexGridVisual (const HexGrid* hg,
                                 const vector<float>& data,
                                 const array<float, 3> offset)
{
    // Copy x/y positions from the HexGrid and make a copy of the data as vertices.
    HexGridVisual* hgv1 = new HexGridVisual(hg, data, offset);
    this->hexGridVis.push_back (hgv1);

    return 0;
}
