#pragma once
#include "constraint.h"
#include <vector>
#include "body.h"
#include "bodyset.h"
#include <armadillo>

namespace morph{ namespace softmats{
/**
 * Shape matching algortithm
 * 
 * This class manages the shape matching constraint on the shape of each body. It keeps
 * a copy of the original shape and computes corrections based upon it for given deformations.
 * TO-DO: Add clusters and other optimizations
 * 
 * @author Alejandro Jimenez Rodriguez
 * @see Müller, M., Heidelberger, B., Teschner, M., & Gross, M. (2005). Meshless deformations based on shape matching. ACM transactions on graphics (TOG), 24(3), 471-478.
 */
    class ShapeMatchingContraint : public Constraint{
    private:
        // ~stiffness
        double alpha;
        // The extent to which stretch is allowed
        double beta;
        // Precomputed inverse
        arma::mat Aqqi;
        // Original center of mass
        arma::vec x0_cm;
        // Transformation
        arma::mat T;
        // Original shape
        std::vector<Point *> shape;
        // A reference to the body whos shape is to be matched
        Body *body;

        // Center of mass
        arma::vec computeCM( bool c);
        // Compute the required transformation for the goal points
        void computeMatrices( arma::vec x_cm );
        // Precomputation of the inverse matrix
        void precompute( );
    public:
        // --- Inherited constraint methods ----
        void init( Body *b );
        void init( BodySet *bs );
        void generate(  int step = 0);
        void solve();
        void updateVelocity();
        void reset();

        ShapeMatchingContraint( double alpha );
        ~ShapeMatchingContraint();
    };

}}