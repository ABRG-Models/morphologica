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
/**
 * Represents an active collision
 * 
 * @author Alejandro Jimenez Rodriguez
 */
class Collision{
public:
    double hc; // Time of contact within the time step
    int ctype; // Collision type. 0 => Face-Point
    bool active; // Whether the collision needs to be processed
    vec cp; // Contact point
    vec cnormal; // Collision normal
    
    Collision( double hc, vec cp, vec cnormal );
    virtual void updateVelocity() = 0;
    virtual void solve(CollisionTest *collisionTest) = 0;
};

/**
 * Represents a Face-point collision
 * @author Alejandro Jimenez Rodriguez
 */
class FPCollision : public Collision{
public:
    Face* f;
    Point* p;
    vector<vec> impulses; // impulses unused
    
    FPCollision( double hc, vec cp, vec cnormal, Face* f, Point* p );
    // Recomputes collision parameters and moves the points towards the contact point
    void solve(CollisionTest *collisionTest);
    // Adds impulses - newton's third law
    void updateVelocity();
};

/**
 * Represents Edge-Edge collision
 *  
 * @author Alejandro Jimenez Rodriguez
  */
class EECollision : public Collision{
public:
    Edge e1;
    Edge e2;
    vector<vec> impulses;

    EECollision( double hc, vec cp, vec cnormal, Edge e1, Edge e2 );
    void solve(CollisionTest *collisionTest);
    void updateVelocity();
};

/**
 * Manages the list of collisions
 * 
 * @au*thor Alejandro Jimenez Rodriguez
 */
class CollisionList{
private:
    
public:
vector<Collision *> collisions;
    CollisionList();

    void push( Collision* c );
    int count();
    bool isEmpty();
    void clear();
    // Removes and returns the first collision in the queue
    Collision* pop();
    void discount( double hc );
};
}}
#endif // COLLISION