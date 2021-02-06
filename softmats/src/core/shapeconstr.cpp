#include "shapeconstr.h"
#include "../util/config.h"

using namespace morph::softmats;

arma::vec ShapeMatchingContraint::computeCM( bool c = false){
    arma::vec cm = arma::zeros<arma::vec>(3);
    double msum = 0;

    for( Point* pt : body->getMesh()->getVertices() ){
        if( c )
            cm += pt->x_c/pt->w;
        else
            cm += pt->x/pt->w;
        msum += 1.0/pt->w;
    }    

    return cm/msum;
}

void ShapeMatchingContraint::precompute( ){
    arma::vec q;
    this->x0_cm = this->computeCM();
    
    this->Aqqi = arma::zeros<arma::mat>(3, 3);

    for( Point* pt : shape ){
        q = pt->x - this->x0_cm;
        this->Aqqi += (1.0/pt->w)*q*q.t();
    }

    this->Aqqi = arma::inv(this->Aqqi);
}

void ShapeMatchingContraint::init( Body *b ){
    for( Point* p : b->getMesh()->getVertices() ){
        Point *q = new Point();
        q->x = p->x; 
        q->v = p->v;
        q->normal = p->normal;
        q->x_c = p->x_c;
        q->w = p->w;
        q->fext = p->fext;
        shape.push_back(q);
    }   

    this->body = b;       
    this->precompute();
}

void ShapeMatchingContraint::computeMatrices( arma::vec x_cm ){
    arma::vec p, q;
    arma::cx_mat S;
    int n = 3;
    arma::mat Apq( n, n, arma::fill::zeros );
    std::vector<Point *>& vert = body->getMesh()->getVertices(); 
    
    for( int i = 0; i < vert.size(); ++i ){        
        p = vert[i]->x_c - x_cm;
        q = shape[i]->x - this->x0_cm;        
        Apq += (1.0/vert[i]->w)*p*q.t();
    }

    S = arma::sqrtmat(Apq.t()*Apq);    
    arma::mat R = arma::real(Apq*S.i());  
    arma::mat A = Apq*this->Aqqi;
    
    // A = A/cbrtf(arma::det(A));
    
    this->T = this->beta*A + (1 - this->beta)*R;
}

void ShapeMatchingContraint::generate( int step ){
// Do nothing
}

void ShapeMatchingContraint::reset(){
// ?
}

void ShapeMatchingContraint::solve(){

    arma::vec x_cm = this->computeCM( true );
    this->computeMatrices( x_cm );
    std::vector<Point *>& vert = body->getMesh()->getVertices(); 
    arma::vec g;
    arma::vec dx;
    double h = Config::getConfig()->getTimeStep();

    for( int i = 0; i < vert.size(); ++i ){        
        g = this->T*(shape[i]->x - this->x0_cm) + x_cm;
        dx = alpha*(g - vert[i]->x_c);
        arma::vec nx = vert[i]->x_c + dx;
        vert[i]->x_c = nx;
    }
}

ShapeMatchingContraint::ShapeMatchingContraint( double alpha ){
    this->alpha = alpha;
    this->beta = 0.0;
}

void ShapeMatchingContraint::updateVelocity(){
    // nothing
}

ShapeMatchingContraint::~ShapeMatchingContraint(){

}

