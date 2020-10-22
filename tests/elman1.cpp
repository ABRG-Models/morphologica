#include <morph/Random.h>
#include "Elman.h"

int main()
{
    // Create an Elman style feed-forward network with context layers
    std::vector<unsigned int> layer_spec = {2,10,2};
    ElmanNet<float> el1(layer_spec);

    return 0;
}
