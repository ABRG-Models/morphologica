#ifndef COLLISION_H
#define COLLISION_H

#include <armadillo>
#include <vector>
#include "../core/point.h"
#include "../core/face.h"
#include "../core/edge.h"
#include "../util/calg.h"
#include "collisiontest.h"

using namespace std;
using namespace arma;

namespace morph{ namespace softmats{
class CollisionTest;

class Collision{
public:
    double hc;
    int ctype;
    bool active;
    vec cp;
    vec cnormal;
    
    Collision( double hc, vec cp, vec cnormal );
    virtual void updateVelocity() = 0;
    virtual void solve(CollisionTest *collisionTest) = 0;
};

class FPCollision : public Collision{
public:
    Face* f;
    Point* p;
    vector<vec> impulses;
    
    FPCollision( double hc, vec cp, vec cnormal, Face* f, Point* p );
    void solve(CollisionTest *collisionTest);
    void updateVelocity();
};

class EECollision : public Collision{
public:
    Edge e1;
    Edge e2;
    vector<vec> impulses;

    EECollision( double hc, vec cp, vec cnormal, Edge e1, Edge e2 );
    void solve(CollisionTest *collisionTest);
    void updateVelocity();
};

class CollisionList{
private:
    
public:
vector<Collision *> collisions;
    CollisionList();

    void push( Collision* c );
    int count();
    bool isEmpty();
    void clear();
    Collision* pop();
    void discount( double hc );
};
}}
#endif // COLLISION