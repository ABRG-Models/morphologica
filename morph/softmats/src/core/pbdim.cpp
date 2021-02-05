#include <armadillo>
#include "point.h"
#include "pbdim.h"
#include "../util/config.h"

using namespace morph::softmats;

PBD::PBD(){
    h = Config::getConfig()->getTimeStep();
}

void PBD::generateConstraints(BodySet *bs, int step ){
    for( Constraint *c : bs->getConstraints() ){
        c->generate( step );
    }

    for( Body *b : bs->getBodies() ){
        for( Constraint *c : b->getConstraints() ){
            c->generate( step );
        }
    }
}

void PBD::velocityUpdate( BodySet *bs ){
    for( Body *b : bs->getBodies() ){
        for( Constraint *c : b->getConstraints() ){
            c->updateVelocity();
        }
    }

    for( Constraint *c : bs->getConstraints() ){
        c->updateVelocity();
    }
}

void PBD::projectConstraints( BodySet *bs ){ 

    for( Constraint *c : bs->getConstraints() ){
        c->solve();
    }
}

void PBD::loop( BodySet *bs, int step ){
    // Advancing body velocity
    for(Body* b : bs->getBodies() ){
        if( b->type == BodyType::GROUND ) continue;
        for( Point* q : b->getMesh()->getVertices() ){
            if( !q->lock )
                q->v += h*q->w*q->fext;
        }
    }

    // Computing candidate positions
    for(Body* b : bs->getBodies() ){
        if( b->type == BodyType::GROUND ) continue;  

        for( Point* q : b->getMesh()->getVertices() )
            if( !q->lock )
                q->x_c = q->x + h*q->v; 
        
        b->getMesh()->computeNormals( true );
    }
    
    // Solving the body constraint (shape matching)
    for( Body *b : bs->getBodies() ){
        for( Constraint *c : b->getConstraints() ){
            c->solve();
        }
    } 
    // Generating collision constraints
    generateConstraints( bs, step );

    // Projecting constraints
    for( int i = 0; i < Config::getConfig()->getNumIterations(); ++i )
        projectConstraints( bs );

    // Update the state to valid positions
    for(Body* b : bs->getBodies() ){  
        if( b->type == BodyType::GROUND ) continue;
             
        for( Point* q : b->getMesh()->getVertices() ){
            if( !(q->lock) ){
                q->v = (q->x_c - q->x)/h;                
                q->x = q->x_c; 
            }
        }

        b->getMesh()->computeNormals( false );
    }

    // Update velocity
    velocityUpdate( bs );
}