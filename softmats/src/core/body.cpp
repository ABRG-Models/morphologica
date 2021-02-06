#include "body.h"
#include "shapeconstr.h"

using namespace morph::softmats;

Body::Body(){
    fext = {0.0, 0.0, 0.0};
}

void Body::addShapeConstraint( Constraint *c ){
    if( c != NULL ){
        ((ShapeMatchingContraint *)c)->init( this );
        constraints.push_back( c );
    }
}

std::vector<Constraint *> Body::getConstraints(){
    return constraints;
}

TriangleMesh* Body::getMesh(){
    return this->mesh;
}

void Body::setMesh( TriangleMesh *mesh ){
    this->mesh = mesh;
}

void Body::setExternalForce( arma::vec f ){
    this->fext = f;
}

void Body::updateReceptors(){
    for( Point* p : getMesh()->getVertices() ){
        p->ground_receptor = false;
    }
}

void Body::resetForces(){
    
    for( Point* p : getMesh()->getVertices() ){

        if( p->w > 0 )
            p->fext = this->fext/p->w;
        else
            p->fext = {0.0, 0.0, 0.0};
    }
}

Body::~Body(){
    delete mesh;
    
    for( Constraint *c : constraints )
        delete c;
}