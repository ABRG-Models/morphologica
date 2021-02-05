#pragma once
#include <iostream>
#include <vector>
#include "collision.h"
#include "../core/body.h"

namespace morph{ namespace softmats{


class Contact{
private:
    Body *A;
    Body *B;
    std::vector<Collision *> collisions;
public:

    Contact( Body *A, Body *B );
    void addCollision( Collision* c );
    Body* getA();
    Body* getB();
    void clearCollisions();
    bool hasCollisions();
    double getContactArea( bool includeFloor );
    void clearInactiveCollisions();
    vector<Collision*>& getCollisions();
};

}}

