#pragma once
#include "constraint.h"
#include <vector>
#include "body.h"
#include "bodyset.h"
#include <armadillo>

namespace morph{ namespace softmats{
    class ShapeMatchingContraint : public Constraint{
    private:
        double alpha;
        double beta;
        arma::mat Aqqi;
        arma::vec x0_cm;
        arma::mat T;
        std::vector<Point *> shape;
        Body *body;

        arma::vec computeCM( bool c);
        void computeMatrices( arma::vec x_cm );
        void precompute( );
    public:
        void init( Body *b );
        void init( BodySet *bs );
        void generate(  int step = 0);
        void solve();
        void updateVelocity();

        ShapeMatchingContraint( double alpha );
        ~ShapeMatchingContraint();
    };

}}