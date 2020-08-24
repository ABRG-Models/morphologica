/*
 * Create a small network with hand-set weights and biases and then run
 * feedforward/backprop a number of times for profiling. Use this with valgrind or perf.
 *
 * \author Seb James
 * \date August 2020
 */

#include <vector>
#include "FeedForward.h"

int main()
{
    // Create a feed-forward network
    std::vector<unsigned int> layer_spec = {2,1000,2};

    // Manually set the input and desired output:
    morph::vVector<float> in = {0.05, 0.0025};
    morph::vVector<float> out = {0.8, 0.95};

    FeedForwardNet<float> ff1(layer_spec, &in);
    ff1.setInput (in, out);

    // Manually set up the weights and biases:
    auto coni = ff1.connections.begin();
    coni->w = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6};
    coni->b = {0.13, 0.12, 0.11};
    coni++;
    coni->w = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6};
    coni->b = {0.13, 0.11};

    for (unsigned int i = 0; i < 20; ++i) {
        ff1.feedforward();
        ff1.computeCost();
        ff1.backprop();
    }

    return 0;
}
