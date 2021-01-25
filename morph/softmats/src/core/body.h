#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "constraint.h"
#include "trianglemesh.h"

namespace morph{ namespace softmats{
    enum BodyType{ ANIMAT, GROUND };
    class Constraint;
    
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
        std::vector<Constraint *> constraints;
    protected:
        TriangleMesh* mesh;
    public:
        Material material;
        
        Body();
        ~Body();
        BodyType type;
        void addConstraint( Constraint *c );
        std::vector<Constraint *> getConstraints();
        TriangleMesh *getMesh();
        void setMesh( TriangleMesh *mesh );
    };
}}