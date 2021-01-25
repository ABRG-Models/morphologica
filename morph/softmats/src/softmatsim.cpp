#include "softmatsim.h"
#include "core/point.h"
#include "collisions/collisionconstr.h"
#include <cmath>

using namespace morph::softmats;

SoftmatSim::SoftmatSim( void (*setup)(SoftmatSim *), 
                        void (*update)(SoftmatSim *), 
                        void(*draw)(SoftmatSim *) ){
    
    this->setup = setup;
    this->update = update;
    this->draw = draw;
    this->solver = new PBD();
    view = new View();
    animats = new BodySet();
    running = true;
    videoRecorder = nullptr;
}

void SoftmatSim::video(){
    videoRecorder = new VideoRecorder(600, 600);
}

AnimatSource* SoftmatSim::animatSource( int n, int period, float x, float y, float z){
    AnimatSource* as = new AnimatSource(n, period, x, y, z);
    sources.push_back(as);
    return as;
}

// Creation members
Animat* SoftmatSim::animat(float x, float y, float z, double mass){
    Animat* a = new Animat(x, y, z);
    a->setMass( mass );
    a->setConstraints();
    a->type = BodyType::ANIMAT;
    std::cout << "Adding animat\n";
    animats->add(a);
    std::cout << "Animat added\n";
    return a;
}

Ground* SoftmatSim::ground( float height ){
    Ground *f = new Ground( height );
    f->type = BodyType::GROUND;
    this->theGround = f;
    std::cout << "Adding the ground\n";
    animats->add(f);
    view->setupGround( f );
    std::cout << "Finished setting up ground\n";
    return f;
}

// AnimatSource* SoftmatSim::source( Point *p ){

// }

// Enviroment control
void SoftmatSim::light( bool v ){

}

void SoftmatSim::gravity( float v ){
    for( Body* a : animats->getBodies() ){
        std::cout << "Setting gravity for " << a << "\n";
        for( Point* p : a->getMesh()->getVertices() ){
            if( p->w > 0 )
                p->fext = {0.0, -fabs(v)/p->w, 0.0};
            else
                p->fext = {0.0, 0.0, 0.0};
            
            p->fext.print();
        }
    }
}

void SoftmatSim::camera( float az, float ev ){
    view->setCamera( az, ev );
}

// Listener
void SoftmatSim::onFinish( void (*f)(const SoftmatSim *s) ){
    this->finishFn = f;
}

void SoftmatSim::onContact( void (*f)(const SoftmatSim *s, const vector<Animat *>& animats) ){
    this->contactFn = f;
}

// void SoftmatSim::onAnimatContact( void (*f)(const Animat* a, vector<Receptor*> receptors) ){
    
// }

// Investigation
// vector<ContactRegion *> contacts();
// bool hasContact( const Animat* a, const Animat* b );
// constexpr ContactRegion *contactRegion( const Animat* a, const Animat *b );

void SoftmatSim::drawAll(){
    view->displayGround();

    for( Body *b : animats->getBodies() ){
        if( b->type == BodyType::ANIMAT ){
            view->displayBody( b );
            std::cout << "Faces body: " << b->getMesh()->getFaces().size() << "\n";
        }
    }
}

void SoftmatSim::initialize(){
    CollisionConstraint *cc = new CollisionConstraint();
    // cc->setCollisionTest(new GroundCollisionTest(theGround));
    cc->setCollisionTest(new ContinuousCollisionTest());
    animats->addConstraint( cc );
}

void SoftmatSim::spawnSources( int step ){
    for( AnimatSource* as : sources ){
        Animat *a = as->getAnimat(step);
        if( a != nullptr ){
            animats->add( a );
            animats->reset();
            gravity(10.0);
        }
    }
}

// Run
void SoftmatSim::run(){
    
    (*setup)( this );  
    initialize(); 
    int step = 0;

    if( videoRecorder != nullptr )
        videoRecorder->setup();

    while( running && !view->shouldClose() ){
        spawnSources(step);

        (*update)(this);
        solver->loop( animats, step );
        
        for( Body *b : animats->getBodies() )
            b->getMesh()->updateVertexNormals();

        view->preDisplay();
        (*draw)(this);
        view->postDisplay();

        if( videoRecorder != nullptr )
            videoRecorder->notify();
        // running = false;
        step++;
    }

    (*finishFn)( this );
    if( videoRecorder != nullptr )
        videoRecorder->notifyEnd();
}

void SoftmatSim::cleanup(){
    delete animats;
    delete solver;
    delete view;
}
    
    
SoftmatSim::~SoftmatSim(){
    cleanup();
}
