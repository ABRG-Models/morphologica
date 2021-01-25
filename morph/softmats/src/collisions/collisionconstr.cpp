#include "collisionconstr.h"
#include "box.h"
#include "../util/timemanager.h"
#include "../util/config.h"

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

void CollisionConstraint::reset(){
	points.clear();
	faces.clear();
	indexes.clear();
	objects.clear();
	collisions->clear();
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

	if( arma::dot( p->x_c - f->points[0]->x_c, f->normal) > 0.1 || 
		cf.body == cp.body ) return;

	FPCollision *fpc = (FPCollision *)collisionTest->testFPCollision(f, p);
	
	if( fpc != nullptr ){
		collisions->push( fpc );		
	}
	
	vector<Edge> pedges = cp.body->getMesh()->getPointEdges( p );
	vector<Edge> fedges = cp.body->getMesh()->getFaceEdges( f );
	EECollision *eec;

	for( Edge& ep : pedges ){
		for( Edge& ef : fedges ){	
			eec = (EECollision *)collisionTest->testEECollision( ep, ef );

			if( eec != nullptr ){
				collisions->push( eec );
			}
		} // For
	}
}


void CollisionConstraint::generate( int step ){
	collisions->clear();


	TimeManager::getInstance()->tic();
    firstPass(step);
	TimeManager::getInstance()->toc();
	TimeManager::getInstance()->tic();
	secondPass(step);
	TimeManager::getInstance()->toc();
	
}

void CollisionConstraint::solve(){

	if( collisions->count() == 0 ) return;

	for( Collision* c : collisions->collisions ){
		if( !c->active ){ std::cout<<"Inactive contact\n"; continue;}

		c->solve( this->collisionTest );
	}

}

void CollisionConstraint::updateVelocity(){

	while( !collisions->isEmpty() ){
		Collision* c = collisions->pop();
		c->updateVelocity();	
	}
}

void CollisionConstraint::setCollisionTest( CollisionTest* test ){
	collisionTest = test;
}

CollisionConstraint::CollisionConstraint():ht(5000,0.2){
	collisions = new CollisionList();
}

CollisionConstraint::~CollisionConstraint(){
	collisions->clear();
	delete collisions;
}