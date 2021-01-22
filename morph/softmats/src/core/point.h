#pragma once
#include <glm/glm.hpp>
#include <armadillo>
#include <vector>
#include <algorithm>

namespace morph{ namespace softmats{

typedef struct Point{
    arma::vec x;
    arma::vec v;
    arma::vec normal;
    arma::vec x_c;
    arma::vec fext;
    glm::vec2 uv;
    
    bool lock;
    double w;
    std::vector<Point *> adj;

    Point(){ 
        this->x = {0.0, 0.0, 0.0};
        this->x_c = {0.0, 0.0, 0.0};
        this->v = {0.0, 0.0, 0.0};
        this->normal = {0.0, 0.0, 0.0};
        this->uv = glm::vec2(0.0f, 0.0f);
        this->lock = false;
    }

    void addAdjacent( Point *p ){
        if( std::find( adj.begin(), adj.end(), p ) == adj.end())
            adj.push_back(p);
    }
} Point;

}}