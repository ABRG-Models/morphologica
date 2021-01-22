#include "collision.h"

using namespace morph::softmats;

// Collision List
CollisionList::CollisionList(){

}

void CollisionList::push( Collision* c ){
    vector<Collision *>::iterator it;
    // cout << "Number of collisions before: " << collisions.size() << endl;
    // if( collisions.empty() )collisions.push_back( c );
    bool exist = false;
    if( c->ctype == 0 ){
        FPCollision *fpc = (FPCollision*)c;

        for( Collision *g : collisions ){
            if( g->ctype == 0 )
                if( ((FPCollision*)g)->f == fpc->f && ((FPCollision*)g)->p == fpc->p )
                    exist = true;
        }
    }

    if( !exist )
        collisions.push_back( c );
    // else{
    //     for( it = collisions.begin(); it != collisions.end(); ++it  ){
    //         if( (*it)->hc > c->hc ){
    //             collisions.insert( it, c );
    //             break;
    //         }
    //     }
    // }

    // cout << "collision list: ";
    // for( it = collisions.begin(); it != collisions.end(); ++it  )
    //     cout << (*it)->hc << ", ";
    // cout << endl;

    // cout << "Number of collisions after: " << collisions.size() << endl;
    // cin.get();
}

void CollisionList::clear(){
    collisions.clear();
}

int CollisionList::count(){
    return collisions.size();
}

bool CollisionList::isEmpty(){
    return collisions.empty();
}

Collision* CollisionList::pop(){
    // cout << "Poping!" << collisions.size() << endl;
    if( !this->isEmpty() ){
        // cout << "Getting collision" << endl;
        Collision *c = this->collisions[0];
        this->collisions.erase( collisions.begin() );

        return c;
    }

    return NULL;
}

void CollisionList::discount( double hc ){
    for( Collision *c : collisions )
        c->hc -= hc;
}

// Collision
Collision::Collision( double hc ):hc(hc){

}

void Collision::updatePointSpeed( Point *p, vec imp ){
	
	
    // p->interactions++;
}

// FPCollision
FPCollision::FPCollision( double hc, Face* f, Point* p, vec w ):Collision(hc),f(f),p(p),w(w){
    this->ctype = 0;
}

void FPCollision::resolve(){
    // set velocities - inelastic collision
	
}

// EECollision
EECollision::EECollision( double hc, Edge e1, Edge e2 ):Collision(hc), e1(e1), e2(e2){
     this->ctype = 1;
}
    
    
void EECollision::resolve(){
    // change velocities - inelastic collision
    
   
}