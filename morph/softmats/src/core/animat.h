#pragma once

#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include "body.h"
#include "point.h"
#include "face.h"

namespace morph{ namespace softmats{

class Animat : public Body{
private:  
    void init(int);    
public:
    Animat( float x, float y, float z );    
    
    void setMass( double m );
    void setConstraints();
};

}}