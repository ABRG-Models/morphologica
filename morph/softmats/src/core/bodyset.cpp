#include "bodyset.h"

using namespace morph::softmats;

void BodySet::add( Body * b ){
    if( b != NULL )
        bodies.push_back(b);
}

std::vector<Body *> BodySet::getBodies(){
    return bodies;
}
        
void BodySet::addConstraint( Constraint *c ){
    if( c != NULL ){
        c->init( this );
        constraints.push_back(c);
    }
}

void BodySet::reset(){
    for( Constraint* c : constraints ){
        c->reset();
        c->init(this);    
    }

}
        
std::vector<Constraint *> BodySet::getConstraints(){
    return constraints;
}
        
BodySet::BodySet(){

}
        
BodySet::~BodySet(){

    for( Body *b : getBodies() ){
        delete b;
    }

    for( Constraint *c : constraints ){
        delete c;
    }
}