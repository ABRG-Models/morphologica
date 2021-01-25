#pragma once
#include "collision.h"
#include "../core/face.h"
#include "../core/point.h"
#include "../core/edge.h"
#include "../core/ground.h"
#include "../util/calg.h"

namespace morph{ namespace softmats{
class Collision;

class CollisionTest{
public:
    virtual Collision *testFPCollision( Face* f, Point *p ) = 0;
    virtual Collision *testEECollision( Edge& ep, Edge& ef ) = 0;
};

class ContinuousCollisionTest : public CollisionTest{
private:

public:
    ContinuousCollisionTest();
    Collision *testFPCollision( Face* f, Point *p );
    Collision *testEECollision( Edge& ep, Edge& ef );
};

class GroundCollisionTest : public CollisionTest{
private:
    double height;
public:
    GroundCollisionTest(Ground *ground);
    Collision *testFPCollision( Face* f, Point *p );
    Collision *testEECollision( Edge& ep, Edge& ef );
};

}}