#include <softmats/src/softmatsim.h>
#include <softmats/src/core/animat.h>
#include <softmats/src/collisions/collision.h>

using namespace morph::softmats;

Animat *a;
Animat *b;

void setup( SoftmatSim *s ){
    std::cout << "Setting up the simulation\n";
    // a = s->animat(-2.0, -1.0, 0.0, 100.0 );
    // b = s->animat(-1.5, 1.1, 0.0, 100.0 );
    AnimatSource *as = s->animatSource(10, 200, 0.0, 2.5, 0.0);
    s->ground( -2.0 );
    s->gravity( 10.0 );
    s->video("bags");
    // s->camera(-0.0, 2.2);

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

void onContact( const SoftmatSim *s, ContactList *contacts ){
    contacts->print();
    std::cout << "Contact area : " << contacts->getContactArea( false ) << "\n";
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