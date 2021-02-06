#include "box.h"

using namespace morph::softmats;

//Box
Box::Box(){

}

Box::Box( vec min, vec max ){
	
    this->min = min;
    this->max = max;
}

bool Box::collide( Box& b ){
    bool ex = this->max(0) > b.min(0) && this->min(0) < this->max(0);
    bool ey = this->max(1) > b.min(1) && this->min(1) < this->max(1);
    bool ez = this->max(2) > b.min(2) && this->min(2) < this->max(2);

    return ex && ey && ez;
}

bool Box::inside( Point *p ){
    return (p->x_c(0) > min(0)) && (p->x_c(1) > min(1)) && (p->x_c(2) > min(2)) &&
           (p->x_c(0) < max(0)) && (p->x_c(1) < max(1)) && (p->x_c(2) < max(2));
}

void Box::compute( vector<Point *>& points, Box *b ){

    vec pmin = points[0]->x_c;
    vec pmax = points[1]->x_c;

    for( Point* pm: points ){
        vec p = pm->x_c;

        if( p(0) < pmin(0) ) pmin(0) = p(0);
        if( p(1) < pmin(1) ) pmin(1) = p(1);
        if( p(2) < pmin(2) ) pmin(2) = p(2);

        if( p(0) > pmax(0) ) pmax(0) = p(0);
        if( p(1) > pmax(1) ) pmax(1) = p(1);
        if( p(2) > pmax(2) ) pmax(2) = p(2);
    }

    b->min = pmin;
    b->max = pmax;

    for(int i = 0; i < 3; i++)
        if( fabs(b->min(i) - b->max(i)) < 1e-3 ){
            b->max(i) += 0.1;
            b->min(i) -= 0.1;
        }
}