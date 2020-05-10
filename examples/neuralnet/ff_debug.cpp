/*
 * Create a small network with hand-set weights and biases to compare with Michael
 * Nielsen's python code. Was used while debugging the code.
 *
 * \author Seb James
 * \date May 2020
 */

#include "Mnist.h"
#include <morph/Random.h>
#include <fstream>
#include "FeedForward.h"

int main()
{
    // Create a feed-forward network
    std::vector<unsigned int> layer_spec = {2,3,2};
    FeedForwardNet<float> ff1(layer_spec);
    // Initial network:
    std::cout << ff1 << std::endl;

    morph::vVector<float> in = {0.05, 0.0025};
    morph::vVector<float> out = {0.8, 0.95};

    ff1.setInput (in, out);

    // Manually set up the weights and biases
    auto coni = ff1.connections.begin();
    coni->w = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6};
    coni->b = {0.13, 0.12, 0.11};
    coni++;
    coni->w = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6};
    coni->b = {0.13, 0.11};

    ff1.feedforward();
    ff1.computeCost();
    ff1.backprop();

    std::cout << ff1 << std::endl;

    return 0;
}
