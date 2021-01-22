#pragma once
#include "../core/body.h"
#include "../core/point.h"
#include "../core/face.h"
#include "box.h"

namespace morph{ namespace softmats{
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

