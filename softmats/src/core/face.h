#pragma once
#include <armadillo>
#include <vector>
#include "point.h"

namespace morph{ namespace softmats{
/**
 * Face data structure
 * 
 * The face has a current normal (normal) and a candidate normal (normal_c)
 * 
 * @author Alejandro Jimenez Rodriguez
 */
    typedef struct Face{
        std::vector<Point*> points;
        arma::vec normal_c;
        arma::vec normal;

        Face( Point *p1, Point* p2, Point* p3 ){
            points.push_back(p1);
            points.push_back(p2);
            points.push_back(p3);
            p1->addAdjacent(p2);
            p1->addAdjacent(p3);
            p2->addAdjacent(p1);
            p2->addAdjacent(p3);
            p3->addAdjacent(p1);
            p3->addAdjacent(p2);
        }
    } Face;

}}