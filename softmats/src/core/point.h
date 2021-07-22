#pragma once
#include <armadillo>
#include <vector>
#include <algorithm>
#include <morph/Vector.h>

namespace morph{ namespace softmats{
/**
 * Point data structure
 *
 * Note that the point has a current position (x) and a candidate position (x_c)
 * to which constraints are applied
 *
 * @author Alejandro Jimenez Rodriguez
 */
typedef struct Point{
    arma::vec x; // current position
    arma::vec v; // current velocity
    arma::vec normal; // current normal
    arma::vec x_c; // candidate position
    arma::vec fext; // external force
    morph::Vector<float, 2> uv; // texture coordinates

    bool lock; // Is the point to be moved?
    double w; // Inverse mass
    double temp_receptor;
    bool ground_receptor;
    std::vector<Point *> adj; // Adjacent points in the mesh

    Point(){
        this->x = {0.0, 0.0, 0.0};
        this->x_c = {0.0, 0.0, 0.0};
        this->v = {0.0, 0.0, 0.0};
        this->normal = {0.0, 0.0, 0.0};
        this->uv = {0.0f, 0.0f};
        this->lock = false;
    }

    void addAdjacent( Point *p ){
        if( std::find( adj.begin(), adj.end(), p ) == adj.end())
            adj.push_back(p);
    }
} Point;

}}
