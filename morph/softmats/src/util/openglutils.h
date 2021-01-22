#pragma once
#ifndef SOFTMATS_OPENGLUTILS_H
#define SOFTMATS_OPENGLUTILS_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <fstream>

using namespace std;
namespace morph{ namespace softmats{
    
class OpenglUtils{
public:
    static float* goldAmbient(){static float a[4] = { 0.2473f, 0.1995f, 0.0745f, 1 }; return (float*)a;}
    static float* goldDiffuse() { static float a[4] = { 0.7516f, 0.6065f, 0.2265f, 1 }; return (float*)a; }
    static float* goldSpecular() { static float a[4] = { 0.6283f, 0.5559f, 0.3661f, 1 }; return (float*)a; }
    static float goldShininess() { return 51.2f; }

    static string readShaderSource( const char* path );

    static void printShaderLog( GLuint shader );

    static void printProgramLog( int prog );

    static GLuint loadTextureChecker( int width, int height );

    static GLuint loadTextureImage( const char* textImagePath );

    static GLuint loadTexture( const void* data, int width, int height );

    static bool checkOpenGLError();

    static GLuint createShaderProgram( const char* vn, const char* fn );

};

}}
#endif