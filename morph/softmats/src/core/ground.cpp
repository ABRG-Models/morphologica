#include <armadillo>
#include <vector>
#include "ground.h"
#include "point.h"
#include "../util/meshutil.h"

using namespace morph::softmats;

Ground::Ground( float height ){
    this->height = height;
    type = BodyType::GROUND;
    init( height );
}

double Ground::getHeight(){
    return height;
}

void Ground::init( float height ){
    PlaneMeshProvider mp;

    mesh = mp.buildMesh();
    mesh->translate( 0, height, 0 );

    for( Point *p : mesh->getVertices() )
        p->w = 0.0;
}
