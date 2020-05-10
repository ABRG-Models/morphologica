#include "Mnist.h"
#include <morph/Random.h>
#include <fstream>
#include "NeuralNet.h"

int main()
{
    // Read the MNIST data
    Mnist m;

    // Instantiate the network (What would be really cool would be a FeedForwardNet as a
    // variadic template, so that FeedForwardNet<float, 785, 15, 10> ff1 would create
    // the network the right way)
    std::vector<unsigned int> layer_spec = {784,30,10};
    FeedForwardNetS<float> ff1(layer_spec);

    // Create a random number generator
    morph::RandUniform<unsigned char> rng((unsigned char)0, (unsigned char)9);

    // main loop parameters are number of epochs, the size of a mini-batch and the
    // learning rate eta
    unsigned int epochs = 30;
    unsigned int mini_batch_size = 10;
    float eta = 3.0f;

    // Accumulate the dC/dw and dC/db values in gradients. for each pair, the first is
    // nabla_w the second is nabla_b. There are as many pairs as there are connections
    // in ff1. Here, we declare an initialize mean_gradients
    std::vector<std::pair<morph::vVector<float>, morph::vVector<float>>> mean_gradients;
    for (auto& c : ff1.connections) {
        mean_gradients.push_back (std::make_pair(c.nabla_w, c.nabla_b));
    }

    // Open a file to output costs into (for making a graph)
    std::ofstream costfile;
    costfile.open ("cost.csv", std::ios::out|std::ios::trunc);

    // i is used as an iterator variable throughout the loop below
    unsigned int i = 0;

    for (unsigned int ep = 0; ep < epochs; ++ep) {

        // At start of epoch, make a copy of the training data:
        std::multimap<unsigned char, morph::vVector<float>> training_f = m.training_f;

        for (unsigned int j = 0; j < training_f.size()/mini_batch_size; ++j) {

            // Update the mini-batch

            // Zero mean grads and cost
            for (i = 0; i < ff1.connections.size(); ++i) {
                mean_gradients[i].first.zero();
                mean_gradients[i].second.zero();
            }
            float cost = 0.0f;

            // Loop through each member of the mini-batch
            for (unsigned int mb = 0; mb < mini_batch_size; ++mb) {

                // Set up input
                auto t_iter = training_f.find (rng.get());
                unsigned int key = static_cast<unsigned int>(t_iter->first);
                morph::vVector<float> thein = t_iter->second;
                morph::vVector<float> theout(10);
                theout.zero();
                theout[key] = 1.0f;
                training_f.erase (t_iter);
                ff1.setInput (thein, theout);

                ff1.compute();
                cost += ff1.computeCost();
                ff1.backprop();
                //std::cout << "After ff1.backprop():\n---------------\n" << ff1 << std::endl;

                // Now collect up the cost, the nabla_w and nabla_bs for the learning step
                i = 0;
                for (auto& c : ff1.connections) {
#if 0
                    std::cout << "Gradients during accumulation:\n";
                    std::cout << "layer " << i << ", nabla_w: " << c.nabla_w << std::endl;
                    std::cout << "      " << i << ", nabla_b: " << c.nabla_b << std::endl;
#endif
                    mean_gradients[i].first += c.nabla_w;
                    mean_gradients[i].second += c.nabla_b;
                    ++i;
                }
            }
#if 0
            std::cout << "Gradients before division (but after accumulation):\n";
            for (i = 0; i < ff1.connections.size(); ++i) {
                std::cout << "layer " << i << ", nabla_w: " << mean_gradients[i].first << std::endl;
                std::cout << "      " << i << ", nabla_b: " << mean_gradients[i].second << std::endl;
            }
#endif
            // Have accumulated cost and mean_gradients, so now divide through to get the means
            for (i = 0; i < ff1.connections.size(); ++i) {
                mean_gradients[i].first /= static_cast<float>(mini_batch_size);
                mean_gradients[i].second /= static_cast<float>(mini_batch_size);
            }
            cost /= static_cast<float>(mini_batch_size);
            costfile << cost << std::endl;
            //std::cout << cost << std::endl;
#if 0
            std::cout << "Gradients after accumulation and division:\n";
            for (i = 0; i < ff1.connections.size(); ++i) {
                std::cout << "layer " << i << ", nabla_w: " << mean_gradients[i].first << std::endl;
                std::cout << "      " << i << ", nabla_b: " << mean_gradients[i].second << std::endl;
            }
            std::cout << "BEFORE gradient alteration ff1:\n---------------\n" << ff1 << std::endl;
#endif

            costfile << cost << "," << mean_gradients[0].first[0] << "," << ff1.connections.front().nabla_w[0] << std::endl;

            // Gradient update. v -> v' = v - eta * gradC
            i = 0;
            for (auto& c : ff1.connections) {
                c.w -= (mean_gradients[i].first * eta);
                c.b -= (mean_gradients[i].second * eta);
                ++i;
            }
#if 0
            std::cout << "After gradient alteration ff1:\n---------------\n" << ff1 << std::endl;
#endif
#if 0
            unsigned int numcorrect = ff1.evaluate (m.test_f, 10);
            std::cout << "In that last minibatch, "<< numcorrect << "/10 were characterized correctly" << std::endl;
            std::cout << ff1;
#endif
        }
#if 1
        // Evaluate for that epoch
        unsigned int numcorrect = ff1.evaluate (m.test_f);
        std::cout << "In that last Epoch, "<< numcorrect << "/10000 were characterized correctly" << std::endl;
        //std::cout << "At epoch end ff1:\n---------------\n" << ff1 << std::endl;
#endif
    }

    costfile.close();

    return 0;
}
