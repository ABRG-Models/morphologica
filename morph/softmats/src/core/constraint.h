#pragma once

namespace morph{ namespace softmats{


class Constraint{
/**
 * General constraint interface for Bodies and BodySets
 * 
 * @author Alejandro Jimenez Rodriguez
 */
public:
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