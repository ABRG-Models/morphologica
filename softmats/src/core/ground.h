#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "body.h"
#include "point.h"
#include "face.h"

namespace morph{ namespace softmats{

/**
 * Ground body
 * 
 * @author Alejandro Jimenez Rodriguez
 */
class Ground: public Body{
private:
    double height;
    void init( float height );
public:    
    Ground( float height );
    double getHeight();
};

}}