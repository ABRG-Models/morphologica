#include <armadillo>
#include <cmath>
#include "animat.h"
#include "shapeconstr.h"
#include "../util/meshutil.h"
#include "../util/openglutils.h"
#include "../util/config.h"

using namespace morph::softmats;

Animat::Animat( float x, float y, float z ){
    init(16);
    type = BodyType::ANIMAT;
    mesh->translate(x, y, z);
    mesh->scale(1.0f);
    material.matAmb = OpenglUtils::goldAmbient();
    material.matDif = OpenglUtils::goldDiffuse();
    material.matSpe = OpenglUtils::goldSpecular();
    material.matShi = OpenglUtils::goldShininess();
}

void Animat::setConstraints(){
    ShapeMatchingContraint *smc = new ShapeMatchingContraint( 0.2 );
    this->addShapeConstraint( smc );
}

void Animat::setMass( double m ){
    double pm = (double)m/mesh->getNumVertices();

    for( Point *p : mesh->getVertices() )
        p->w = 1/pm;
}

void Animat::init( int prec ){
    // SphereMeshProvider mp( SphereMeshProvider::TYPICAL );
    ObjMeshProvider mp( Config::getConfig()->getMeshLocation().c_str() );
    mesh = mp.buildMesh();
}

void Animat::addGroundImpulse( arma::vec f ){
    for( Point* p : getMesh()->getVertices() ){
        if( p->ground_receptor )
            p->fext += f;
    }
}

void Animat::move(  double x, double y, double z ){
    arma::vec dir = {x, y, z};
    dir /= norm(dir);
    addGroundImpulse( 10*dir );
}
