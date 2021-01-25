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
    std::cout << "advancing velocity\n";
    for(Body* b : bs->getBodies() ){
        if( b->type == BodyType::GROUND ) continue;
        for( Point* q : b->getMesh()->getVertices() ){
            if( !q->lock )
                q->v += h*q->w*q->fext;
        }
    }

    std::cout << "Generating candidate pos\n";
    for(Body* b : bs->getBodies() ){
        if( b->type == BodyType::GROUND ) continue;  

        for( Point* q : b->getMesh()->getVertices() )
            if( !q->lock )
                q->x_c = q->x + h*q->v; 
        
        b->getMesh()->computeNormals( true );
    }
    
     for( Body *b : bs->getBodies() ){
        for( Constraint *c : b->getConstraints() ){
            c->solve();
        }
    } 
    std::cout << "Generating constraints\n";
    generateConstraints( bs, step );

    // std::cout << "Projecting constrints\n";
    for( int i = 0; i < Config::getConfig()->getNumIterations(); ++i )
        projectConstraints( bs );

    std::cout << "Updating state\n";
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

    std::cout << "Updating velocities\n";
    velocityUpdate( bs );
}