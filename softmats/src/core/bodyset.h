#pragma once
#include <vector>
#include "constraint.h"
#include "../collisions/contactlist.h"
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
        arma::vec fext;
        std::vector<Constraint *> constraints;
        std::vector<Body *> bodies;
    public:

        void add( Body* b );
        std::vector<Body *> getBodies();
        void addCollisionConstraint( Constraint *c );
        std::vector<Constraint *> getConstraints();
        bool hasContacts();
        ContactList* getContacts();
        void reset();
        void addExternalForce( arma::vec f );
        void resetForces();
        void resetReceptors();
        BodySet();
        ~BodySet();
    };

}}