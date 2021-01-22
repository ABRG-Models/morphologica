#pragma once
#include "../core/constraint.h"
#include "../core/body.h"
#include "../core/bodyset.h"
#include "../core/point.h"
#include "../core/face.h"

#include "chashtable.h"
#include "collisiondstruct.h"
#include "collision.h"

namespace morph{ namespace softmats {

class CollisionConstraint : public Constraint{
private:
    vector<CPoint> points;
	vector<CFace> faces;
	map<Body*, vector<int>> indexes;
	map<Body*, int> objects;
	// Hash tables
	CHashTable ht;
    BodySet *bodySet;
    CollisionList *collisions;
    // Hashes points and computes aabb
	void firstPass( int step ); 
	// Check the faces and handles collisions
	void secondPass( int step );
    void solveProximity( CFace& cf, CPoint& cp );
	void evaluateContacts( CFace& cf, int step );
    void handleCollisions( CFace& cf, CHashItem chi, int step );
    void storeCollision( CFace& cf, CPoint& cp );
    
public:
    void init( Body *b );
    void init( BodySet *bs );
    void registerObject( Body *b );
    void generate( int step = 0 );
    void solve();
    void updateVelocity();

    CollisionConstraint();
    ~CollisionConstraint();
};

}}
