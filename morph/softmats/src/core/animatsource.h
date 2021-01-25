#include "animat.h"
#include <armadillo>

namespace morph{ namespace softmats{
/**
 * Spawns sources at a given rate (1/period)
 * 
 * @author Alejandro Jimenez Rodriguez
 */
class AnimatSource{
private:
    int n;
    int period;
    int count;
    arma::vec pos;
public:
    AnimatSource( int numAnimats, int period, float x, float y, float z );

    /**
     * Returns a new Animat if the step is a multiple of the period
     * or null otherwise
     */
    Animat* getAnimat( int step );
};

}}