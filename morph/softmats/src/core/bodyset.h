#pragma once
#include <vector>
#include "constraint.h"
#include "body.h"

namespace morph{ namespace softmats{
    class Constraint;
    class Body;

    class BodySet{
    private:
        std::vector<Constraint *> constraints;
        std::vector<Body *> bodies;
    public:

        void add( Body* b );
        std::vector<Body *> getBodies();
        void addConstraint( Constraint *c );
        std::vector<Constraint *> getConstraints();
        BodySet();
        ~BodySet();
    };

}}