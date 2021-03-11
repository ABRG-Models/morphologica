#ifndef CHASHTABLE_H
#define CHASHTABLE_H

#include <map>
#include <list>
#include <iostream>
#include <armadillo>
#include <vector>
#include "box.h"

using namespace std;
using namespace arma;

namespace morph{ namespace softmats{

typedef struct {
    list<int> items;
    int timestamp;
} CHashItem;

/**
CHashTable - Implements a hash table for the search of collisions
             in a given volume.
*/
class CHashTable{

    const unsigned long p1 = 73856093;
    const unsigned long p2 = 19349663;
    const unsigned long p3 = 83492791;
public:
    int n;
    double l; // Cell size
    CHashTable( int n, double l );
    vector<CHashItem> hashes;

    int discretize( double a );
    void discretizeBox( Box *b );
    unsigned int getHash( vec point );
    unsigned int getHashDiscrete( vec p );
    void hashIn( vec point, int index, int step );
    CHashItem getItem( unsigned int h );

};

}}
#endif // CHASHTABLE_H
