#pragma once
#include "../core/body.h"
namespace morph{ namespace softmats{

class View{
public:
    virtual void setupGround( Body *ground) = 0;
    
    virtual void preDisplay() = 0;  
    
    virtual void displayGround() = 0;

    virtual bool shouldClose() = 0;
    
    virtual void displayBody( Body* b ) = 0;
    
    virtual void postDisplay() = 0;
    
    virtual void setCamera(float az, float ev) = 0;
};
}}