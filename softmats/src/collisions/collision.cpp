#include "collision.h"
#include "contactlist.h"
#include "contact.h"

using namespace morph::softmats;

Contact::Contact(Body* A, Body* B ):A(A), B(B){
}

Body* Contact::getA(){
    return A;
}

Body* Contact::getB(){
    return B;
}

double Contact::getContactArea( bool includeFloor ){
    if( !includeFloor && (A->type==BodyType::GROUND || B->type == BodyType::GROUND ) )
        return 0.0;

    return collisions.size();
}

bool Contact::hasCollisions(){
    return !collisions.empty();
}

void Contact::addCollision( Collision* c  ){
    vector<Collision *>::iterator it;
    
    bool exist = false;

    if( c->ctype == 0 ){
        FPCollision *fpc = (FPCollision*)c;

        for( Collision *g : collisions ){
            if( g->ctype == 0 )
                if( ((FPCollision*)g)->f == fpc->f && ((FPCollision*)g)->p == fpc->p )
                    exist = true;
        }
    }

    // Not adding edge collisions!

    if( !exist )
        collisions.push_back( c );
}

void Contact::updateReceptors(){
    Body *ground = nullptr;
    Body *animat = nullptr;

    if( A->type == BodyType::GROUND && B->type == BodyType::ANIMAT ){
        ground = A;
        animat = B;
    }else if( B->type == BodyType::GROUND && A->type == BodyType::ANIMAT ){
        ground = B;
        animat = A;
    }

    if( ground != nullptr ){
        for( Point *p : animat->getMesh()->getVertices() )
            p->ground_receptor = true;
    }
}

vector<Collision*>& Contact::getCollisions(){
    return collisions;
}

void Contact::print(){
    std::cout << "Contact: A = " << A->getId() << 
                 ", B = " << B->getId() << 
                 ". Collisions: " << getCollisions().size() << "\n";
}
// Collision List
ContactList::ContactList(){

}

void ContactList::print(){
    std::cout << "List of contacts ("<< contacts.size() << "): \n";

    for( Contact *c : contacts ){
        if( c != nullptr )
            c->print();
        else
            std::cerr << "Null contact!\n";
    }
}

Contact *ContactList::findContact( Body *A, Body *B ){
	for( Contact *fc : contacts )
		if( (fc->getA() == A && fc->getB() == B) ||
			(fc->getB() == A && fc->getA() == B))
			return fc;

	return NULL;
}

void Contact::clearCollisions(){
    collisions.clear();
}

void Contact::clearInactiveCollisions(){
    vector<Collision*>::iterator it;

    for( it = collisions.begin(); it != collisions.end(); ){
        if( !(*it)->active )
            it = collisions.erase(it);
        else
            ++it;
    }
}

void ContactList::prune(){
    vector<Contact*>::iterator it;
    for( it = contacts.begin(); it != contacts.end(); ){
        Contact *c = (*it);
        c->clearInactiveCollisions();

        if( !c->hasCollisions() )
            it = contacts.erase(it);
        else
            ++it;
    }
}

double ContactList::getContactArea( bool includefloor ){
    double area = 0.0;

    for( Contact *c : contacts ){
        if( c != nullptr )
            area += c->getContactArea( false );
    }

    return area;
}

void ContactList::push( Body* A, Body* B, Collision* c ){
    
    Contact *contact = findContact(A, B);

	if( contact == NULL ){
		contact = new Contact( A, B );
        contacts.push_back(contact);
	}

	contact->addCollision( c );

    
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

void ContactList::clear(){
    for( Contact *c : contacts )
        c->clearCollisions();

    contacts.clear();
}

int ContactList::count(){
    return contacts.size();
}

bool ContactList::isEmpty(){
    return contacts.empty();
}

void ContactList::updateReceptors(){
    for( Contact *c : contacts ){
        c->updateReceptors();
    }
}

// Collision* ContactList::pop(){
//     // cout << "Poping!" << collisions.size() << endl;
//     if( !this->isEmpty() ){
//         // cout << "Getting collision" << endl;
//         Collision *c = this->collisions[0];
//         this->collisions.erase( collisions.begin() );

//         return c;
//     }

//     return NULL;
// }
vector<Contact*>& ContactList::getContacts(){
    return contacts;
}

// Collision
Collision::Collision( double hc, vec cp, vec cnormal ):hc(hc), cp(cp), cnormal(cnormal), active(true){
}

// FPCollision
FPCollision::FPCollision( double hc, vec cp, vec cnormal, Face* f, Point* p ):Collision(hc, cp, cnormal),f(f),p(p){
    this->ctype = 0;
}

void FPCollision::solve(CollisionTest *collisionTest){

    Collision* c = collisionTest->testFPCollision( this->f, this->p );

    if( c != nullptr ){
        this->cp = c->cp;
        this->hc = c->hc;
        
        double cr = 1.0;
        double cc = arma::dot(p->x_c - cp, f->normal_c);

        if( cc < 0 ){
            if( p->w > 0 && !p->lock )
                p->x_c += -cc*cr*f->normal_c;
            // p->x_c = cp;

            for( Point *q : f->points){
                if( q->w > 0 && !q->lock ){
                    q->x_c -= -cc*cr*f->normal_c;
                }
            }
        }

        delete c;
    }
	
}

void FPCollision::updateVelocity(){
    arma::vec vel, fvec;
	double cr = 0.01;
    vel = p->v;
    vel -= 2.0 * cr * arma::dot(vel, f->normal_c) * f->normal_c;
    fvec = -(vel - arma::dot(vel, f->normal_c) * f->normal_c);
    
    if( p->w > 0 && !p->lock )
        p->v += fvec;

    for( Point *q : f->points){
        if( q->w > 0 && !q->lock )
            q->v -= fvec;
    }
}

// EECollision
EECollision::EECollision( double hc, vec cp, vec cnormal, Edge e1, Edge e2 ):Collision(hc,cp,cnormal), e1(e1), e2(e2){
     this->ctype = 1;
}
    
    
void EECollision::solve(CollisionTest *collisionTest){
    // change velocities - inelastic collision
    
    Collision* c = collisionTest->testEECollision( this->e1, this->e2 );

    if( c != nullptr ){
        // updating collision
        this->cp = c->cp;
        this->hc = c->hc;
        this->cnormal = c->cnormal;

        Point *p1 = e1.p1;
        Point *p2 = e1.p2;
        Point *q1 = e2.p1;
        Point *q2 = e2.p2;
        double cr = 1.0;
        double cc1 = arma::dot(p1->x_c - cp, cnormal);
        double cc2 = arma::dot(p2->x_c - cp, cnormal);
        double cc3 = arma::dot(q1->x_c - cp, cnormal);
        double cc4 = arma::dot(q2->x_c - cp, cnormal);

        if( p1->w > 0 && !p1->lock )
            p1->x_c += -cc1*cr*cnormal;
        if( p2->w > 0 && !p2->lock )
            p2->x_c += -cc2*cr*cnormal;
        if( q1->w > 0 && !q1->lock )
            q1->x_c -= -cc3*cr*cnormal;
        if( q2->w > 0 && !q2->lock )
            q2->x_c -= -cc4*cr*cnormal;
    }
}

void EECollision::updateVelocity(){
    Point *p1 = e1.p1;
    Point *p2 = e1.p2;
    Point *q1 = e2.p1;
    Point *q2 = e2.p2;

    arma::vec vel, fvec;
	double cr = 1.0;
    
    if( p1->w > 0 && !p1->lock ){
        vel = p1->v;
        vel -= 2.0 * cr * arma::dot(vel, cnormal)*cnormal;
        fvec = -(vel - arma::dot(vel, cnormal)*cnormal);    
        p1->v += fvec;
    }

    if( p2->w > 0 && !p2->lock ){
        vel = p2->v;
        vel -= 2.0 * cr * arma::dot(vel, cnormal)*cnormal;
        fvec = -(vel - arma::dot(vel, cnormal)*cnormal);    
        p2->v += fvec;
    }

    if( q1->w > 0 && !q1->lock ){
        vel = q1->v;
        vel -= 2.0 * cr * arma::dot(vel, cnormal)*cnormal;
        fvec = -(vel - arma::dot(vel, cnormal)*cnormal);    
        q1->v += fvec;
    }

    if( q2->w > 0 && !q2->lock ){
        vel = q2->v;
        vel -= 2.0 * cr * arma::dot(vel, cnormal)*cnormal;
        fvec = -(vel - arma::dot(vel, cnormal)*cnormal);    
        q2->v += fvec;
    }
    
}