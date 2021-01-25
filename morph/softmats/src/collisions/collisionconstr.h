#pragma once
#include "../core/constraint.h"
#include "../core/body.h"
#include "../core/bodyset.h"
#include "../core/point.h"
#include "../core/face.h"

#include "chashtable.h"
#include "collisiondstruct.h"
#include "collision.h"
#include "collisiontest.h"

namespace morph{ namespace softmats {
/**
 * Collision detection and response managing.
 * The spatial scanning is performed efficiently with spatial hashing. The response and update are implemented separatedly
 * 
 * @author Alejandro Jimenez Rodriguez
 * @see Teschner, M., Heidelberger, B., Müller, M., Pomerantes, D., & Gross, M. H. (2003, November). Optimized spatial hashing for collision detection of deformable objects. In Vmv (Vol. 3, pp. 47-54).
 */
class CollisionConstraint : public Constraint{
private:
    // Collection of all the points in the simulation
    vector<CPoint> points;
    // Collection of all the faces in the simulation
	vector<CFace> faces;
    // unused data structures atm
	map<Body*, vector<int>> indexes;
	map<Body*, int> objects;
	// Hash tables
	CHashTable ht;
    // The corresponding body set
    BodySet *bodySet;
    // List of all active collisions
    CollisionList *collisions;
    // Current collision testing strategy being used
    CollisionTest *collisionTest;
    // Hashes points and computes aabb
	void firstPass( int step ); 
	// Check the faces and handles collisions
	void secondPass( int step );
    // Unused
    void solveProximity( CFace& cf, CPoint& cp );
    // Spatial hashing specific methods
	void evaluateContacts( CFace& cf, int step );
    void handleCollisions( CFace& cf, CHashItem chi, int step );
    void storeCollision( CFace& cf, CPoint& cp );
    
public:
    // Inherited constraints methods
    void init( Body *b );
    void init( BodySet *bs );
    void generate( int step = 0 );
    void solve();
    void updateVelocity();
    void reset();
    // Add the body points to the ongoing collection of points for detection
    void registerObject( Body *b );
    void setCollisionTest( CollisionTest* test );

    CollisionConstraint();
    ~CollisionConstraint();
};

}}
