#include "collisiontest.h"
#include "../util/config.h"
#include <armadillo>

using namespace morph::softmats;

// Continuous collision testing
ContinuousCollisionTest::ContinuousCollisionTest(){

}

Collision* ContinuousCollisionTest::testFPCollision( Face* f, Point *p ){
    double h = Config::getConfig()->getTimeStep();
	double delta = 1e-20;
	vector<Point *>& fpoints = f->points;
	vec x1 = fpoints[0]->x;
	vec x2 = fpoints[1]->x;
	vec x3 = fpoints[2]->x;
	vec x4 = p->x;
	vec v1 = (fpoints[0]->x_c - fpoints[0]->x)/h;
	vec v2 = (fpoints[1]->x_c - fpoints[1]->x)/h;
	vec v3 = (fpoints[2]->x_c - fpoints[2]->x)/h;
	vec v4 = (p->x_c - p->x)/h;

	PolyData data = {x1, x2, x3, x4, v1, v2, v3, v4};
	
	int nb = 10;
    float tb1[20], tb2[20];
    zbrak( data, 0.0, h, 20, tb1, tb2, &nb );

    if( nb == 0 ){
		return nullptr;
	}

	float t1, t2, rt = 10000;

	for( int i = 0; i < nb; i++ ){
		t1 = tb1[i];
		t2 = tb2[i];
		rt = rtflsp( data, t1, t2, 1e-10);
		
		vec u1 = x1 + rt*v1;
		vec u2 = x2 + rt*v2;
		vec u3 = x3 + rt*v3;
		vec y =  x4 + rt*v4;
		vec nt = arma::cross(u1-u2, u1-u3);
		nt /= arma::norm(nt);

		vec w = computeBarycentricCoords( u1, u2, u3, y );

		if( fabs(arma::dot(nt, y-u1)) < h/1000.0 && allInInterval( w, -delta, 1-delta) ){
            double hc = rt;
			vec cp;

			if( rt < 1e-2 )
				cp = x4;
			else 
				cp = y;

			return new FPCollision(hc, cp, f->normal_c, f, p);
		}
		
	}

	return nullptr;	
}

Collision* ContinuousCollisionTest::testEECollision( Edge& ep, Edge& ef ){
	double h = Config::getConfig()->getTimeStep();
	// if( h < 1e-6 )
	// 	{cout << "step too small!"<< endl;cin.get();}
	double delta = 1e-6;
	vec x1 = ep.p1->x;
	vec x2 = ep.p2->x;
	vec x3 = ef.p1->x;
	vec x4 = ef.p2->x;
	vec v1 = (ep.p1->x_c - x1)/h;
	vec v2 = (ep.p2->x_c - x2)/h;
	vec v3 = (ef.p1->x_c - x3)/h;
	vec v4 = (ef.p2->x_c - x4)/h;

	vec n = computeEdgeNormal(x1, x2, x3, x4 );
	PolyData data = {x1, x2, x3, x4, v1, v2, v3, v4};

	int nb = 3;
    float tb1[3], tb2[3];
    zbrak( data, 0.0, h, 30, tb1, tb2, &nb );
    // cout << "Potential roots:" << nb << endl;

    if( nb == 0 ) return nullptr;

	float t1, t2, rt = 10000;
	
	for( int i = 0; i < nb; i++ ){
		t1 = tb1[i];
		t2 = tb2[i];
		rt = rtflsp( data, t1, t2, 1e-10);
		
		double d = computeEdgeDistance( x1 + rt*v1, x2 + rt*v2, x3 + rt*v3, x4 + rt*v4 );
		double hc = rt;
		vec cp = x4 + rt*v4;
		if( d < delta ) return new EECollision(hc, cp, n, ep, ef);
	}
	
	return nullptr;	
}

// Ground collision test
GroundCollisionTest::GroundCollisionTest(Ground *ground){
    height = ground->getHeight();
}

Collision* GroundCollisionTest::testFPCollision( Face* f, Point *p ){
    if( p->x_c(1) < height ){
        vec pp = {0, height, 0};
        vec cp = {p->x_c(0), height, p->x_c(2)};
		
        return new FPCollision(-10000, cp, f->normal_c, f, p);
    }

    return nullptr;
}

Collision* GroundCollisionTest::testEECollision( Edge& ep, Edge& ef ){
    vec pp = {0, height, 0};
    vec cp;

    // Trivial

    return nullptr;
}