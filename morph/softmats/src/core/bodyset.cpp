#include "bodyset.h"
#include <iostream>
#include "../collisions/collisionconstr.h"

using namespace morph::softmats;

void BodySet::add( Body * b ){
    if( b != NULL )
        bodies.push_back(b);
}

std::vector<Body *> BodySet::getBodies(){
    return bodies;
}
        
void BodySet::addCollisionConstraint( Constraint *c ){
    if( c != NULL ){
        ((CollisionConstraint *)c)->init( this );
        constraints.push_back(c);
    }
}

bool BodySet::hasContacts(){
    if( !constraints.empty() ){
        try{
            CollisionConstraint *cc = (CollisionConstraint *)constraints[0];
            return !cc->getContacts()->isEmpty();
        }catch(std::exception& e){
            std::cerr << "Was not expecting other type of constraints at the moment\n";
        }
    }

    return false;
}

ContactList* BodySet::getContacts(){
    if( !constraints.empty() ){
        try{
            CollisionConstraint *cc = (CollisionConstraint *)constraints[0];
            return cc->getContacts();
        }catch(std::exception& e){
            std::cerr << "Was not expecting other type of constraints at the moment\n";
        }
    }

    return nullptr;
}

void BodySet::reset(){
    for( Constraint* c : constraints ){
        c->reset();
        ((CollisionConstraint *)c)->init(this);    
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