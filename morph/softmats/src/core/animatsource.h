#include "animat.h"
#include <armadillo>

namespace morph{ namespace softmats{

class AnimatSource{
private:
    int n;
    int period;
    int count;
    arma::vec pos;
public:
    AnimatSource( int numAnimats, int period, float x, float y, float z );
    Animat* getAnimat( int step );
};

}}