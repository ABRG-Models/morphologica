#include "collisionconstr.h"
#include "box.h"
#include "../util/timemanager.h"

using namespace morph::softmats;

void CollisionConstraint::init( Body *b ){
// Do nothing
}

void CollisionConstraint::init( BodySet *bs ){
    bodySet = bs;

	for( Body* b : bs->getBodies() )
		// if( b->type == BodyType::ANIMAT )
			registerObject( b );
}

void CollisionConstraint::registerObject( Body *b ){
	std::cout << "Registering object for collision detection\n";
	vector<int> objIdxs;
	vector<Point*>& gPoints = b->getMesh()->getVertices();
	vector<Face*>& gFaces = b->getMesh()->getFaces();
	// Put all the pointers to points in the corresponding vector
	for( int i = 0; i < gPoints.size(); i++ ){
		
		objIdxs.push_back( this->points.size() );
		CPoint cp = {.point = gPoints[i], 
					 .body = b, 
					 .originalIdx = i};
		this->points.push_back( cp );
	}
	
	for( int i = 0; i < gFaces.size(); i++ ){
		CFace cf = {.face = gFaces[i],
					.body = b, 
					.aabb = new Box(),
					.originalIdx = i};
		this->faces.push_back( cf );
	}

	this->indexes[b] = objIdxs;
	this->objects[b] = this->objects.size();
	std::cout << "Object registered\n";
}

void CollisionConstraint::firstPass( int step ){

	for( int i = 0; i < this->points.size(); i++ ){
		// Add hash
		CPoint cp = this->points[i];
		Point* p = cp.point;
		vector<int> idxs = indexes[cp.body]; 
		// cout << "step: " << step << "\n";
		this->ht.hashIn( cp.point->x_c, i, step );
	}

	// Compute the bounding boxes
	for( CFace& cf : this->faces ){
		Box::compute(cf.face->points, cf.aabb );
	}
}

void CollisionConstraint::secondPass( int step ){
	cout << "Faces: " << faces.size() << " \n";
	for( int i = 0; i < this->faces.size(); i++ ){		
		CFace cf = this->faces[i];
		this->evaluateContacts( cf, step );
	}

	cout << "Done second pass\n";
}

void CollisionConstraint::evaluateContacts( CFace& cf, int step ){
	unsigned int h;
	// Discretizing the AABB
	this->ht.discretizeBox(cf.aabb);	

	// Hashing the cells
	for( int kx = (int)cf.aabb->min(0); kx <= cf.aabb->max(0); kx++ ){
		for( int ky = (int)cf.aabb->min(1); ky <= cf.aabb->max(1); ky++ ){
			for( int kz = (int)cf.aabb->min(2); kz <= cf.aabb->max(2); kz++ )
			{
				
				vec p = {kx, ky, kz};				
				h = ht.getHashDiscrete( p );
				CHashItem chi = ht.getItem( h );

				if( chi.timestamp == step ){					
					this->handleCollisions( cf, chi, step );
				}
			}
		}
	}

}

void CollisionConstraint::handleCollisions( CFace& cf, CHashItem chi, int step ){
	
	for( list<int>::iterator it = chi.items.begin(); it != chi.items.end(); ++it ){
		CPoint cp = this->points[*it];
		Point* p = cp.point;		
		
		if( cp.body == cf.body ){continue;}
		this->storeCollision( cf, cp );
	}
	
}

void CollisionConstraint::storeCollision( CFace& cf, CPoint& cp ){
	
	Point* p = cp.point;
	Face* f = cf.face;
	vec pd = {ht.discretize(p->x(0)), ht.discretize(p->x(1)), ht.discretize(p->x(2))};

	// if( !cf.aabb->inside( p ))
	// 	return;
	// 	Check for face-point collisions
	vec w;
	double hc;
	bool colliding = isColliding( cf, cp, &w, &hc, 0.01 );
	
	if( colliding ){
		std::cout << "FPCollision!\n";
		std::cout << "Face: " << f << "\n";
		std::cout << "Point: " << p << "\n";
		if( cf.body->type == BodyType::ANIMAT ){
			cout << "one animat\n";
			cin.get();
		}
		// p->lock = true;
		FPCollision *fpc = new FPCollision( hc, f, p, w );
		fpc->impulses = getInelasticImpulses( f, p, &w );
		collisions->push( fpc );		
	}
	
	vector<Edge> pedges = cp.body->getMesh()->getPointEdges( p );
	vector<Edge> fedges = cp.body->getMesh()->getFaceEdges( f );

	for( Edge& ep : pedges ){
		for( Edge& ef : fedges ){	
			colliding = isColliding( ep, ef, &hc, 0.01 );

			if( colliding ){
				EECollision* eec = new EECollision( hc, ep, ef );
				eec->impulses = getInelasticImpulses( ep, ef );
				collisions->push( eec );
			}
		} // For
	}
}


void CollisionConstraint::generate( int step ){
	std::cout << "Generating constraint\n";
	std::cout << "First pass\n";
	collisions->clear();
	TimeManager::getInstance()->tic();
    firstPass(step);
	TimeManager::getInstance()->toc();
	std::cout << "Second pass\n";
	TimeManager::getInstance()->tic();
	secondPass(step);
	TimeManager::getInstance()->toc();
	std::cout << "Constraint generated " << collisions->count() << " collisions detected\n";
	
}

void CollisionConstraint::solve(){
	double h = 0.01;

	if( collisions->count() == 0 ) return;

	for( Collision* c : collisions->collisions ){
		if( c->ctype == 0 ){ // FPCollision
			Point *p = ((FPCollision*)c)->p;
			Face *f = ((FPCollision*)c)->f;

			// vec v = (p->x_c - p->x)/h;
			p->x_c = p->x + 0.8*c->hc*p->v;

			for( Point *q : f->points){
				q->x_c = q->x + 0.5*c->hc*q->v;
			}
		}else{
			cin.get();
			Point *p1 = ((EECollision *)c)->e1.p1;
			Point *p2 = ((EECollision *)c)->e1.p2;
			Point *q1 = ((EECollision *)c)->e2.p1;
			Point *q2 = ((EECollision *)c)->e2.p2;

			p1->x_c = p1->x + c->hc*p1->v;
			// p1->lock = true;
			p2->x_c = p2->x + c->hc*p2->v;
			// p2->lock = true;
			q1->x_c = q1->x + c->hc*q1->v;
			// q1->lock = true;
			q2->x_c = q2->x + c->hc*q2->v;
			// q2->lock = true;
		}
	}
}

void CollisionConstraint::updateVelocity(){
	cout <<"Updating velocities\n";
	while( !collisions->isEmpty() ){
		Collision* c = collisions->pop();
		vector<vec> impulses;

		if( c->ctype == 0 ){ // FPCollision
			Point *p = ((FPCollision*)c)->p;
			Face *f = ((FPCollision*)c)->f;
			vec wp =  ((FPCollision*)c)->w;

			p->v += ((FPCollision*)c)->impulses[3];

			for( int i = 0; i < f->points.size(); ++i ){
				f->points[i]->v += ((FPCollision*)c)->impulses[i];
			}
		}else{
			cin.get();
			Point *p1 = ((EECollision *)c)->e1.p1;
			Point *p2 = ((EECollision *)c)->e1.p2;
			Point *q1 = ((EECollision *)c)->e2.p1;
			Point *q2 = ((EECollision *)c)->e2.p2;

			p1->v += ((EECollision *)c)->impulses[0];
			p2->v += ((EECollision *)c)->impulses[1];
			p2->v += ((EECollision *)c)->impulses[2];
			p2->v += ((EECollision *)c)->impulses[3];
		}	
	}

	cout << "Done velocity update\n";
}

CollisionConstraint::CollisionConstraint():ht(5000,0.2){
	collisions = new CollisionList();
}

CollisionConstraint::~CollisionConstraint(){
	collisions->clear();
	delete collisions;
}