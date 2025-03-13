/*
 * Create a small network with hand-set weights and biases to compare with Michael
 * Nielsen's python code. Was used while debugging the code.
 *
 * \author Seb James
 * \date May 2020
 */

#include <morph/Mnist.h>
#include <fstream>
#include <morph/vvec.h>
#include <morph/nn/FeedForwardNet.h>

int main()
{
    // Create a feed-forward network
    std::vector<unsigned int> layer_spec = {2,3,2};
    morph::nn::FeedForwardNet<float> ff1(layer_spec);

    // Manually set the input and desired output:
    morph::vvec<float> in = {0.05, 0.0025};
    morph::vvec<float> out = {0.8, 0.95};
    ff1.setInput (in, out);

    // Manually set up the weights and biases:
    auto coni = ff1.connections.begin();
    coni->ws[0] = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6};
    coni->b = {0.13, 0.12, 0.11};
    coni++;
    coni->ws[0] = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6};
    coni->b = {0.13, 0.11};

    std::cout << "\n\nBEFORE feedforward/backprop\n---------------------------\n";
    std::cout << ff1 << std::endl;

    ff1.feedforward();
    ff1.computeCost();
    ff1.backprop();

    std::cout << "\n\nAFTER feedforward/backprop\n---------------------------\n";
    std::cout << ff1 << std::endl;

    return 0;
}
