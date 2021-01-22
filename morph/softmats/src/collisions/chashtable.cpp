#include "chashtable.h"

using namespace morph::softmats;

// CHashTable
CHashTable::CHashTable( int n, double l ):hashes(n){
	this->n = n;
	this->l = l;
	this->hashes.reserve(n);
}

int CHashTable::discretize( double a ){
	int d = floor((a)/this->l);
	return d;
}

unsigned int CHashTable::getHash( vec point ){
	
	int x = discretize(point(0));
	int y = discretize(point(1));
	int z = discretize(point(2));
	vec p = {x, y, z};
	return this->getHashDiscrete(p);
	
}

unsigned int CHashTable::getHashDiscrete( vec p ){
	unsigned long h;
	unsigned long x = p(0);
	unsigned long y = p(1);
	unsigned long z = p(2);

	unsigned long t1 = (x*this->p1);
	unsigned long t2 = (y*this->p2) ;
	unsigned long t3 = (z*this->p3);

	h = (t1 ^ t2 ^ t3)%((unsigned long)this->n);

	return (unsigned int)h;
}

void CHashTable::hashIn( vec point, int index, int step ){
	unsigned int h = getHash( point );

	if( this->hashes[h].timestamp != step ){
		this->hashes[h].items.clear();
		this->hashes[h].timestamp = step;
	}

	this->hashes[h].items.insert( this->hashes[h].items.end(), index );
}

void CHashTable::discretizeBox( Box *b ){
	//cout << "Discretizing box - min: " << printvec(b->min) << ", max: "<<printvec(b->max)<<endl;
	b->min(0) = discretize(b->min(0));
	b->min(1) = discretize(b->min(1));
	b->min(2) = discretize(b->min(2));

	b->max(0) = discretize(b->max(0));
	b->max(1) = discretize(b->max(1));
	b->max(2) = discretize(b->max(2));
}

CHashItem CHashTable::getItem( unsigned int h ){
	return hashes[h];
}