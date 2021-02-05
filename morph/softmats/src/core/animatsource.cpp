#include "animatsource.h"
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

using namespace morph::softmats;

AnimatSource::AnimatSource( int numAnimats, int period, float x, float y, float z ):n(numAnimats),period(period){
    srand (time(NULL));
    this->pos = {x, y, z};
    count = 0;
}

Animat* AnimatSource::getAnimat( int step ){
    if( step%period == 0 && count < n ){
        float mass = 200;
        double rx = 2*rand()%100/100.0;
        double rz = rand()%100/200.0;
        Animat* a = new Animat(pos(0) + rx, pos(1), pos(2) + rz);
        a->setMass( mass );
        a->setConstraints();
        a->type = BodyType::ANIMAT;
        count++;
        return a;
    }
    
    return nullptr;
}