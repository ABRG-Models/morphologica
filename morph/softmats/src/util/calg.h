#ifndef COMPALG_H
#define COMPALG_H

#include <iostream>
#include <vector>
#include <armadillo>

#include "../core/point.h"
#include "../core/edge.h"
#include "../core/face.h"
#include "../collisions/collisiondstruct.h"

using namespace arma;
using namespace std;

namespace morph{
namespace softmats{

struct PolyData{
    vec x1;
    vec x2;
    vec x3;
    vec x4;
    vec v1;
    vec v2;
    vec v3;
    vec v4;
};

// -------
// Root finding functions
void zbrak( PolyData& data, float x1, float x2, int n, float xb1[], float xb2[], int *nb );
float rtflsp( PolyData& data, float x1, float x2, float xacc );
vec clamp( vec x1, vec x2, vec x3, vec x4, vec c, vec *p1, vec *p2 );
// -------
// Collision and contact detection & response
vector<vec> getInelasticImpulses( Face* f, Point* p, vec *w );
vector<vec> getInelasticImpulses( Edge&ep, Edge &ef );
// vector<vec> getCollisionImpulses( CFace& f, CPoint& p, vec *wp );
// vector<vec> getCollisionImpulses( Edge& ep, Edge& ef );
// -------
// Utility computational geometry functions
vec centroid( vector<Point *>& points );
vec computeBarycentricCoords( vec p1, vec p2, vec p3, vec pos );
vec normalCoefficients( vec x1, vec x2, vec x3, vec x4 );
double collision_poly(double t, vec x1, vec x2, vec x3, vec x4, vec v1, vec v2, vec v3, vec v4 );
vec computeEdgeNormal( vec x1, vec x2, vec x3, vec x4 );
double computeEdgeDistance( vec x1, vec x2, vec x3, vec x4 );
bool allInInterval( vec w, double a, double b );
}}

#endif