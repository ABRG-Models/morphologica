#pragma once
#include "../core/body.h"
#include "../core/point.h"
#include "../core/face.h"
#include "box.h"

namespace morph{ namespace softmats{

/**
 * Supporting data structures for the collision detection
 * 
 * @author Alejandro Jimenez Rodriguez
 */
    typedef struct CPoint {
        Point* point;
        Body *body;
        int originalIdx;
    } CPoint;

    typedef struct CFace {
        Face* face;
        Body *body;
        Box *aabb;
        int originalIdx;
    } CFace;
}}

