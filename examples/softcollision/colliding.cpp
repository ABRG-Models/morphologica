#include <softmats/src/softmatsim.h>
#include <softmats/src/core/animat.h>
#include <softmats/src/collisions/collision.h>

/**
 * Two softmats approaching each other and colliding
 */

using namespace morph::softmats;

Animat *a;
Animat *b;

void setup( SoftmatSim *s ){
    a = s->animat(-2.0, -1.0, 0.0, 100.0 );
    b = s->animat(2.0, -1.0, 0.0, 100.0 );
    s->ground( -2.0 );
    s->gravity( 10.0 );
    s->video("colliding_softmats");
    // s->camera(-0.0, 2.2);    
}

void update( SoftmatSim *s ){   
    a->move(1.0, 0.0, 0.0);
    b->move(-1.0, 0.0, 0.0);
}

void draw( SoftmatSim *s  ){
    s->drawAll();
}

void onFinish( const SoftmatSim *s ){
    std::cout<< "Simulation finished\n";
}

void onContact( const SoftmatSim *s, ContactList *contacts ){
    contacts->print();
    std::cout << "Contact area: " << contacts->getContactArea( false ) << "\n";
}

// void onAnimatContact( const Animat* a, vector<Receptor*> receptors ){
//     std::cout << "Animat contact!\n";
// }

int main( int n, char** args ){
    if (n < 2) {
        std::cerr << "Usage: " << args[0] << " /path/to/params.json [/path/to/logdir]" << std::endl;
        return 1;
    }

    // Loading configuration
    std::string pfile = args[1];
    SoftmatSim sim( pfile, &setup, &update, &draw );
    sim.onFinish( &onFinish );
    sim.onContact( &onContact );
    sim.run();
    return EXIT_SUCCESS;
}