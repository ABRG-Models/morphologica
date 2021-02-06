#pragma once
#include <vector>
#include "bodyset.h"

namespace morph{ namespace softmats{
/**
 * Position based dynamics loop 
 * 
 * Implements the general PBD loop. Bodies have local constraints, BodySets have global constraints
 * 
 * @author Alejandro Jimenez Rodriguez
 * @see Bender, J., Müller, M., & Macklin, M. (2017). A survey on position based dynamics, 2017. In Proceedings of the European Association for Computer Graphics: Tutorials (pp. 1-31).
 */
class PBD{
private:
    double h;
public:
    PBD();
    // Generates/initializes the constraints fof bodies and body sets
    void generateConstraints( BodySet *bs, int step );
    // Updates the velocities when required
    void velocityUpdate( BodySet *bs );
    // Updates positions to satisfy constrataints
    void projectConstraints( BodySet *bs );    
    // Main solver loop
    void loop( BodySet *bs, int step );

};

}}