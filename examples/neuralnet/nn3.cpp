#include "Mnist.h"
#include <morph/Random.h>
#include <fstream>
#include "NeuralNet.h"

int main()
{
    // Instantiate the network (What would be really cool would be a FeedForwardNet as a
    // variadic template, so that FeedForwardNet<float, 785, 15, 10> ff1 would create
    // the network the right way)
    std::vector<unsigned int> layer_spec = {2,3,2};
    FeedForwardNetS<float> ff1(layer_spec);
    std::cout << ff1 << std::endl;

    // main loop, while m.training_f has values in:
    unsigned int epochs = 1000; // For now this is 'number of mini batches'
    unsigned int mini_batch_size = 10;
    float eta = 0.1;
    float cost = 0.0f;

    // Accumulate the dC/dw and dC/db values in graidents. for each pair, the first
    // is nabla_w the second is nabla_b. There are as many pairs as there are
    // connections in ff1.
    std::vector<std::pair<morph::vVector<float>, morph::vVector<float>>> mean_gradients;
    // Init mean gradients
    for (auto& c : ff1.connections) {
        mean_gradients.push_back (std::make_pair(c.nabla_w, c.nabla_b));
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
        for (i = 0; i < ff1.num_connection_layers(); ++i) {
            mean_gradients[i].first.zero();
            mean_gradients[i].second.zero();
        }

        cost = 0.0f;
        for (unsigned int mb = 0; mb < mini_batch_size; ++mb) {

            morph::vVector<float> thein = {0.1, 0.3};
            morph::vVector<float> theout = {1, 0};

            ff1.setInput (thein, theout);
            ff1.compute();
            cost += ff1.computeCost();
            ff1.backprop();
            //std::cout << "After ff1.backprop():\n---------------\n" << ff1 << std::endl;

            // Now collect up the cost, the nabla_w and nabla_bs for the learning step
            i = 0;
            for (auto& c : ff1.connections) {
                mean_gradients[i].first += c.nabla_w;
                mean_gradients[i].second += c.nabla_b;
                ++i;
            }
        }

        std::cout << "Before division (after accumulation):\n";
        for (i = 0; i < ff1.num_connection_layers(); ++i) {
            std::cout << "layer " << i << ", nabla_w: " << mean_gradients[i].first << std::endl;
            std::cout << "      " << i << ", nabla_b: " << mean_gradients[i].first << std::endl;
        }

        // Have accumulated cost and mean_gradients, so now divide through to get the means
        for (i = 0; i < ff1.num_connection_layers(); ++i) {
            mean_gradients[i].first /= static_cast<float>(mini_batch_size);
            mean_gradients[i].second /= static_cast<float>(mini_batch_size);
        }
        cost /= (2.0f*static_cast<float>(mini_batch_size));
        costfile << cost << std::endl;

        std::cout << "After division:\n";
        for (i = 0; i < ff1.num_connection_layers(); ++i) {
            std::cout << "layer " << i << ", nabla_w: " << mean_gradients[i].first << std::endl;
            std::cout << "      " << i << ", nabla_b: " << mean_gradients[i].first << std::endl;
        }

        std::cout << "BEFORE gradient alteration ff1:\n---------------\n" << ff1 << std::endl;

        // Gradient update. v -> v' = v - eta * gradC
        i = 0;
        for (auto& c : ff1.connections) {
            c.w -= (mean_gradients[i].first * eta);
            c.b -= (mean_gradients[i].second * eta);
            ++i;
        }

        std::cout << "After gradient alteration ff1:\n---------------\n" << ff1 << std::endl;
    }

    costfile.close();

    return 0;
}
