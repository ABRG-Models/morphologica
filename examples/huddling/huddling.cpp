#include <softmats/src/softmatsim.h>
#include <softmats/src/core/animat.h>
#include <softmats/src/collisions/collision.h>
#include <vector>
#include <armadillo>

using namespace morph::softmats;

float Ta = 0.5;
float h = 0.01;

struct Pup{
    Animat *animat;
    float Tb;
    float A;
    float G;
    float Tp;
    float theta;
    float Tc;
    float Tr;
    float Tl;

    Pup(){
        Tb = 0.0;
        A = 1.0;
        G = 0.0;
        Tp = 0.6;
        theta = 0.0;
    }
};

std::vector<Pup> pups;

void evolvePup( Pup& pup ){
    pup.Tb += h*(pup.A*(Ta - pup.Tb) - (1 - pup.A)*(pup.Tc - pup.Tb) + pup.G);
    pup.theta += h*(pup.Tr - pup.Tl)*(pup.Tb - pup.Tp);
}

void generatePups( SoftmatSim *s, int n ){
    float w = 1.2;
    float l = sqrt(n);
    float x = -l*w/2.0;
    float z = -l*w/2.0;

    for( int i = 0; i < n; i++ ){
        Pup p;
        p.animat = s->animat(x, -1.5, z, 100.0);
        pups.push_back( p );
        x += w;

        if( x > l*w/2.0 ){
            x = -l*w/20;
            z += w;
        }
    }
}

void processPupContact( Pup& pa, Pup& pb, Contact* c ){
    pa.Tl = 0.0;
    pa.Tr = 0.0;
    pa.Tc = 0.0;
    pb.Tl = 0.0;
    pb.Tr = 0.0;
    pb.Tc = 0.0;

    for( Collision *col : c->getCollisions() ){
        arma::vec cpa = col->cp - centroid(pa.animat->getMesh()->getVertices());
        // Determining half space of the point for pa
        arma::vec fa = {cosf(pa.theta), 0, sinf(pa.theta)};
        arma::vec ofa = {fa(0), 0, -fa(2)};
        float dira = arma::dot( ofa, cpa );
        // Adding Tl or Tl and Tc
        if( dira < 0 )
            pa.Tl += (pa.Tb + pb.Tb)/2.0;
        else
            pa.Tr += (pa.Tb + pb.Tb)/2.0;
        // Repeat for Pb
        arma::vec cpb = col->cp - centroid(pb.animat->getMesh()->getVertices());
        // Determining half space of the point for pa
        arma::vec fb = {cosf(pb.theta), 0, sinf(pb.theta)};
        arma::vec ofb = {fb(0), 0, -fb(2)};
        float dirb = arma::dot( ofb, cpb );
        // Adding Tl or Tl and Tc
        if( dirb < 0 )
            pb.Tl += (pb.Tb + pa.Tb)/2.0;
        else
            pb.Tr += (pa.Tb + pb.Tb)/2.0;
    }

    pa.Tc = (pa.Tl + pa.Tr)/2.0;
    pb.Tc = (pb.Tl + pb.Tr)/2.0;
}

void setup( SoftmatSim *s ){
    std::cout << "Setting up the simulation\n";
    generatePups( s, 9 );
    s->ground( -2.0 );
    s->gravity( 10.0 );
    // s->video("huddling");
    // s->camera(-0.0, 2.2);

    // v[0].lock = true;
    // s.lights(true);

}

void update( SoftmatSim *s ){
    float x, z;

    for( Pup& p : pups ){
        evolvePup( p );

        x = cosf(p.theta);
        z = sinf(p.theta);
        p.animat->move(x, 0, z);
    }
}

void draw( SoftmatSim *s  ){
    //std::cout << "Drawing\n";
    s->drawAll();
}

void onFinish( const SoftmatSim *s ){
    std::cout<< "Simulation finished\n";
}

void onContact( const SoftmatSim *s, ContactList *contacts ){
#if 0
    for( Contact* c: contacts->getContacts() ){
        // Pup &pa = 0, &pb = 0;

        // for( Pup& p: pups ){
        //     if( c->getA() == p.animat )
        //         &pa = p;
        //     else if( c->getB() == p.animat )
        //         &pb = p;
        // }

        // processPupContact( pa, pb, c );
    }
#endif
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
