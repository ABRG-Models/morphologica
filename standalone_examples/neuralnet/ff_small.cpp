/*
 * Train a small feedforward neural network on some simple data points. This was a
 * trial program to help develop ff_mnist, which uses a much larger (and more
 * difficult to debug) network.
 *
 * \author Seb James
 * \date May 2020
 */

#include <morph/Mnist.h>
#include <morph/Random.h>
#include <morph/nn/FeedForwardNet.h>
#include <morph/vvec.h>
#include <fstream>
#include <vector>
#include <utility>

int main()
{
    // Create a feed-forward network
    std::vector<unsigned int> layer_spec = {2,3,2}; // 2,2,2, 2,5,2; 2,4,2; 2,3,2. 2,11,2 all work
    morph::nn::FeedForwardNet<float> ff1(layer_spec);
    std::cout << ff1 << std::endl;

    // 5 data points from the function I want to find. This converts a quadratic input to a ~linear output
    std::vector<morph::vvec<float>> ins = {{0.05, 0.0025}, {0.2, 0.04}, {0.4, 0.16}, {0.6, 0.36}, {0.8, 0.64}};
    std::vector<morph::vvec<float>> outs = {{0.8, 0.95}, {0.6, 0.7}, {0.4, 0.5}, {0.2, 0.2}, {0.05, 0.05}};

    // main loop, while m.training_f has values in:
    unsigned int epochs = 2000; // Here, an epoch is a run through each batch of 5 in/outs.
    unsigned int mini_batch_size = ins.size();
    float eta = 0.5;
    float cost = 0.0f;

    // Accumulate the dC/dw and dC/db values in graidents. for each pair, the first
    // is nabla_w the second is nabla_b. There are as many pairs as there are
    // connections in ff1.
    std::vector<std::pair<morph::vvec<float>, morph::vvec<float>>> mean_gradients;
    // Init mean gradients
    for (auto& c : ff1.connections) {
        mean_gradients.push_back (std::make_pair(c.nabla_ws[0], c.nabla_b));
    }
    for (auto g : mean_gradients) {
        std::cout << "nabla_w: " << g.first << ", nabla_b: " << g.second << std::endl;
    }

    std::ofstream costfile;
    costfile.open ("cost.csv", std::ios::out|std::ios::trunc);

    // Used throughout as an iterator variable
    unsigned int i = 0;
    for (unsigned int ep = 0; ep < epochs; ++ep) {

        // Zero mean grads
        for (i = 0; i < ff1.connections.size(); ++i) {
            mean_gradients[i].first.zero();
            mean_gradients[i].second.zero();
        }

        cost = 0.0f;
        for (unsigned int mb = 0; mb < mini_batch_size; ++mb) {

            // Note: NOT stochastic!
            morph::vvec<float> thein = ins[mb];
            morph::vvec<float> theout = outs[mb];

            ff1.setInput (thein, theout);
            ff1.feedforward();
            cost += ff1.computeCost();
            ff1.backprop();
            //std::cout << "After ff1.backprop():\n---------------\n" << ff1 << std::endl;

            // Now collect up the cost, the nabla_w and nabla_bs for the learning step
            i = 0;
            for (auto& c : ff1.connections) {
                mean_gradients[i].first += c.nabla_ws[0];
                mean_gradients[i].second += c.nabla_b;
                ++i;
            }
        }
#if 0
        std::cout << "Before division (after accumulation):\n";
        for (i = 0; i < ff1.connections.size(); ++i) {
            std::cout << "layer " << i << ", nabla_w: " << mean_gradients[i].first << std::endl;
            std::cout << "      " << i << ", nabla_b: " << mean_gradients[i].first << std::endl;
        }
#endif
        // Have accumulated cost and mean_gradients, so now divide through to get the means
        for (i = 0; i < ff1.connections.size(); ++i) {
            mean_gradients[i].first /= static_cast<float>(mini_batch_size);
            mean_gradients[i].second /= static_cast<float>(mini_batch_size);
        }
        cost /= (static_cast<float>(mini_batch_size));
        costfile << cost << std::endl;
#if 0
        std::cout << "After division:\n";
        for (i = 0; i < ff1.connections.size(); ++i) {
            std::cout << "layer " << i << ", nabla_w: " << mean_gradients[i].first << std::endl;
            std::cout << "      " << i << ", nabla_b: " << mean_gradients[i].first << std::endl;
        }
        std::cout << "BEFORE gradient alteration ff1:\n---------------\n" << ff1 << std::endl;
#endif

        // Gradient update. v -> v' = v - eta * gradC
        i = 0;
        for (auto& c : ff1.connections) {
            c.ws[0] -= (mean_gradients[i].first * eta);
            c.b -= (mean_gradients[i].second * eta);
            ++i;
        }
#if 0
        std::cout << "After gradient alteration ff1:\n---------------\n" << ff1 << std::endl;
#endif

    }
    costfile.close();

    ff1.evaluate (ins, outs);
    std::cout << ff1;

    return 0;
}
