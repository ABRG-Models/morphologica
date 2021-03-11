#pragma once
#include "collision.h"
#include "../core/face.h"
#include "../core/point.h"
#include "../core/edge.h"
#include "../core/ground.h"
#include "../util/calg.h"

namespace morph{ namespace softmats{
class Collision;

/**
 * Different kinds of tests for determining collisions between 
 * primitives.
 * 
 * @author Alejandro Jimenez Rodriguez
 */
class CollisionTest{
public:
    virtual Collision *testFPCollision( Face* f, Point *p ) = 0;
    virtual Collision *testEECollision( Edge& ep, Edge& ef ) = 0;
};

/**
 * Continuous tests find the exact point in the ray x -> x_c 
 * at which two moving primitives intersect.
 * 
 * @author Alejandro Jimenez Rodriguez
 */
class ContinuousCollisionTest : public CollisionTest{
private:

public:
    ContinuousCollisionTest();
    Collision *testFPCollision( Face* f, Point *p );
    Collision *testEECollision( Edge& ep, Edge& ef );
};

/**
 * Ground collision testing to be using when no other animats are in the simulation
 * 
 * @author Alejandro Jimenez Rodriguez
 */
class GroundCollisionTest : public CollisionTest{
private:
    double height;
public:
    GroundCollisionTest(Ground *ground);
    Collision *testFPCollision( Face* f, Point *p );
    Collision *testEECollision( Edge& ep, Edge& ef );
};

}}