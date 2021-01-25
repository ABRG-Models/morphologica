#pragma once
#include "body.h"
#include "bodyset.h"

namespace morph{ namespace softmats{
class Body;
class BodySet;

class Constraint{
/**
 * General constraint interface for Bodies and BodySets
 * 
 * @author Alejandro Jimenez Rodriguez
 */
public:
    // Initializes the constraint once
    virtual void init( Body *b ) = 0;
    virtual void init( BodySet* bs ) = 0;
    // Sets up the constraint on each loop
    virtual void generate( int step = 0 ) = 0;
    // Compute corrections derived from the constraint
    virtual void solve() = 0;
    // Update velocities when required
    virtual void updateVelocity() = 0;
    // Resets the constraint
    virtual void reset() = 0;
};

}}