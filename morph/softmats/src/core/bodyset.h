#pragma once
#include <vector>
#include "constraint.h"
#include "body.h"

namespace morph{ namespace softmats{
    class Constraint;
    class Body;
 /**
 * Stores a group of bodies in the simulation
 * 
 * @author Alejandro Jimenez Rodriguez
 */
    class BodySet{
    private:
        std::vector<Constraint *> constraints;
        std::vector<Body *> bodies;
    public:

        void add( Body* b );
        std::vector<Body *> getBodies();
        void addConstraint( Constraint *c );
        std::vector<Constraint *> getConstraints();
        void reset();
        BodySet();
        ~BodySet();
    };

}}