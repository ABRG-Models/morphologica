#pragma once
#ifndef SOFTMATS_VIEW_H
#define SOFTMATS_VIEW_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>
#include <cmath>
#include "../util/openglutils.h"
#include "../core/body.h"

#define numVAOs 1
#define numVBOs 6

namespace morph{ namespace softmats{
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
        glm::vec3 currentPos;
        glm::vec3 posV;
        float pos[3];
        glm::vec3 initialLightLoc;
        // White light
        float globalAmbient[4];
        float lightAmbient[4];
        float ligthDiffuse[4];
        float lightSpecular[4];
    };

    struct Camera{
        float x;
        float y; 
        float z;
    };

    struct ViewPort{
        GLuint mvLoc;
        GLuint prLoc;
        float x;
        float y;
        float z;
        int width;
        int height;
        float aspect;
        glm::mat4 pMat;
        glm::mat4 vMat;
    };

    class View{
    private:    
        GLFWwindow* window;
        GLuint renderingProgram;
        GLuint vao[numVAOs], vbo[numVBOs];
        Camera camera;
        ViewPort viewPort;
        Light light;
        GLuint typeLoc, nLoc;        
        glm::mat4 mMat, mvMat, tMat, rMat, sMat, invTrMat;
        GLuint textureId;
    public:
        void init();
        void setup();
        void setupGround( Body *ground);
        bool shouldClose();

        void preDisplay();
        void displayGround();
        void displayBody( Body* b );
        void postDisplay();
        void setCamera(float az, float ev);

        void installLights( Body*b, glm::mat4 vMatrix );
        
        View();
        ~View();
    }; 

}}

#endif