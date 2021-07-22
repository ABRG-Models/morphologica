#pragma once
#ifndef SOFTMATS_VIEW_H
#define SOFTMATS_VIEW_H

#include <GLFW/glfw3.h>
#include <morph/Vector.h>
#include <morph/TransformMatrix.h>
#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>
#include <cmath>
#include "../util/openglutils.h"
#include "../core/body.h"
#include "view.h"

#define numVAOs 1
#define numVBOs 6

namespace morph{ namespace softmats{
/**
 * Opengl rendering for the softmats
 *
 * This class controls all the visualization aspects of the simulation
 *
 * @author Alejandro Jimenez Rodriguez
 */

    // Light properties
    struct Light{
        GLuint globalAmbLoc;
        GLuint ambLoc;
        GLuint diffLoc;
        GLuint specLoc;
        GLuint posLoc;
        GLuint mAmbLoc;
        GLuint mDiffLoc;
        GLuint mSpecLoc;
        GLuint mShiLoc;
        morph::Vector<float, 3> currentPos;
        morph::Vector<float, 3> posV;
        //float pos[3]; // posV.data() can do what pos[] does
        morph::Vector<float, 3> initialLightLoc;
        // White light
        float globalAmbient[4];
        float lightAmbient[4];
        float ligthDiffuse[4];
        float lightSpecular[4];
    };

    // Viewport data
    struct ViewPort{
        GLuint mvLoc;
        GLuint prLoc;
        float x;
        float y;
        float z;
        int width;
        int height;
        float aspect;
        morph::TransformMatrix<float> pMat;
        morph::TransformMatrix<float> vMat;
    };

    class SoftmatsView : public View{
    private:
        GLFWwindow* window;
        GLuint renderingProgram;
        GLuint vao[numVAOs], vbo[numVBOs];
        // camera position
        morph::Vector<float, 3> camera;
        ViewPort viewPort;
        Light light;
        // Matrices and reserved locations in the shader
        GLuint typeLoc, nLoc;
        morph::TransformMatrix<float> mMat, mvMat, tMat, rMat, sMat, invTrMat; // morph::TransformMatrix
        GLuint textureId;
    public:
        // Initializes the glfw window and opengl contexts
        void init();
        // Sets up the VAOs and VBOs
        void setup();
        // Sets up the static vertices for the ground
        void setupGround( Body *ground);
        // Returns true if the window has been invalidated
        bool shouldClose();

        // Sets up the matrices and other properties common to all entities
        void preDisplay();
        // Sends the ground to the GPU
        void displayGround();
        // Updates and sends a body vertices
        void displayBody( Body* b );
        // Finish up the display
        void postDisplay();
        // Sets the camera position - TO IMPROVE
        void setCamera(float az, float ev);
        // Sets up the lights
        void installLights( Body*b, morph::TransformMatrix<float>& vMatrix );

        SoftmatsView();
        ~SoftmatsView();
    };

}}

#endif
