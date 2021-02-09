#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "constraint.h"
#include "trianglemesh.h"

namespace morph{ namespace softmats{

    enum BodyType{ ANIMAT, GROUND };
    
    // Material information used for lighting
    struct Material{
        float* matAmb;
        float* matDif;
        float* matSpe;
        float matShi;
    };

 /**
 * Parent class for all the bodies (Animats and Ground)
 * 
 * @author Alejandro Jimenez Rodriguez
 */

    class Body{
    private:
        arma::vec fext;
        std::vector<Constraint *> constraints;
        int id;
    protected:
        TriangleMesh* mesh;
    public:
        Material material;
        
        Body();
        ~Body();
        BodyType type;
        void addShapeConstraint( Constraint *c );
        std::vector<Constraint *> getConstraints();
        TriangleMesh *getMesh();
        void setMesh( TriangleMesh *mesh );
        void setExternalForce( arma::vec f );    
        void resetForces();
        void updateReceptors();
        int getId();
        void setId( int id );
    };
}}