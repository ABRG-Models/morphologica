#ifndef COLLISION_H
#define COLLISION_H

#include <armadillo>
#include <vector>
#include "../core/point.h"
#include "../core/face.h"
#include "../core/edge.h"
#include "../util/calg.h"

using namespace std;
using namespace arma;

namespace morph{ namespace softmats{

class Collision{
public:
    double hc;
    int ctype;

    Collision( double hc );
    void updatePointSpeed( Point *p, vec imp );
    virtual void resolve() = 0;
};

class FPCollision : public Collision{
public:
    Face* f;
    Point* p;
    vec w;
    vector<vec> impulses;
    
    FPCollision( double hc, Face* f, Point* p, vec w );
    void resolve();
};

class EECollision : public Collision{
public:
    Edge e1;
    Edge e2;
    vector<vec> impulses;

    EECollision( double hc, Edge e1, Edge e2 );
    void resolve();
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