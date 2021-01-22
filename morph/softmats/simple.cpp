#include "src/softmatsim.h"
#include "src/core/animat.h"

using namespace morph::softmats;

Animat *a;
Animat *b;

void setup( SoftmatSim *s ){
    std::cout << "Setting up the simulation\n";
    a = s->animat(-2.0, 0.5, 0.0, 100.0 );
    b = s->animat(2.0, 0.5, 0.0, 100.0 );
    s->ground( -2.0 );
    s->gravity( 10.0 );
    // s->camera(-0.0, 2.2);

    // std::vector<Point>& v = a->getVertices();
    // v[0].lock = true;
    // s.lights(true);
    
}

void update( SoftmatSim *s ){   
   //std::cout << "Updating the simulation\n";

}

void draw( SoftmatSim *s  ){
    //std::cout << "Drawing\n";
    s->drawAll();
}

void onFinish( const SoftmatSim *s ){
    std::cout<< "Simulation finished\n";
}

void onContact( const SoftmatSim *s, const vector<Animat *>& animats ){
    std::cout << "Contact\n";
}

// void onAnimatContact( const Animat* a, vector<Receptor*> receptors ){
//     std::cout << "Animat contact!\n";
// }

int main( int n, char** args ){

    SoftmatSim sim( &setup, &update, &draw );
    sim.onFinish( &onFinish );
    sim.onContact( &onContact );
    sim.run();
    return EXIT_SUCCESS;
}