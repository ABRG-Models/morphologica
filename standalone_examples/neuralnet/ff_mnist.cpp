/*
 * Train a neural network to characterise the MNIST database of numerals.
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
#include <map>


int main()
{
    // Read the MNIST data
    morph::Mnist m;

    // Instantiate the network
    morph::nn::FeedForwardNet<float> ff1({784,30,10});

    // Create a random number generator
#ifdef __WIN__
    morph::RandUniform<unsigned short> rng((unsigned short)0, (unsigned short)9);
#else
    morph::RandUniform<unsigned char> rng((unsigned char)0, (unsigned char)9);
#endif
    // main loop parameters are number of epochs, the size of a mini-batch and the
    // learning rate eta
    unsigned int epochs = 30;
    unsigned int mini_batch_size = 10;
    float eta = 3.0f;

    // Accumulate the dC/dw and dC/db values in gradients. for each pair, the first is
    // nabla_w the second is nabla_b. There are as many pairs as there are connections
    // in ff1. Here, we declare an initialize mean_gradients
    std::vector<std::pair<morph::vvec<float>, morph::vvec<float>>> mean_gradients;
    for (auto& c : ff1.connections) {
        mean_gradients.push_back (std::make_pair(c.nabla_ws[0], c.nabla_b));
    }

    // Open a file to output costs into (for making a graph)
    std::ofstream costfile;
    costfile.open ("cost.csv", std::ios::out|std::ios::trunc);

    // i is used as an iterator variable throughout the loop below
    unsigned int i = 0;

    for (unsigned int ep = 0; ep < epochs; ++ep) {

        // At start of epoch, make a copy of the training data:
//      std::multimap<unsigned char,                morph::vvec<float>  > training_f = m.training_f;
        std::multimap<unsigned char, std::pair<int, morph::vvec<float>> > training_f = m.training_f;

        unsigned int jj = training_f.size()/mini_batch_size;
        for (unsigned int j = 0; j < jj; ++j) {

            // Learn from one mini-batch...

            // Zero the mean gradents and the cost variable.
            for (i = 0; i < ff1.connections.size(); ++i) {
                mean_gradients[i].first.zero();
                mean_gradients[i].second.zero();
            }
            float cost = 0.0f;

            // Loop through each member of the mini-batch
            for (unsigned int mb = 0; mb < mini_batch_size; ++mb) {

                // Set up input
                auto t_iter = training_f.find (static_cast<unsigned char>(rng.get() & 0xff));
                // Might have run out of that kind of image, so need this:
                if (t_iter == training_f.end()) {
                    while (t_iter == training_f.end()) {
                        t_iter = training_f.find (static_cast<unsigned char>(rng.get() & 0xff));
                    }
                }
                unsigned int key = static_cast<unsigned int>(t_iter->first);
                morph::vvec<float> thein = t_iter->second.second;
                morph::vvec<float> theout(10);
                theout.zero();
                theout[key] = 1.0f;
                training_f.erase (t_iter);
                ff1.setInput (thein, theout);

                // Feedforward then back-propagate errors
                ff1.feedforward();
                cost += ff1.computeCost();
                ff1.backprop();

                // Now collect up the nabla_w and nabla_bs for the learning step (already summed up cost)
                i = 0;
                for (auto& c : ff1.connections) {
                    mean_gradients[i].first += c.nabla_ws[0];
                    mean_gradients[i].second += c.nabla_b;
                    ++i;
                }
            }

            // Have accumulated cost and mean_gradients, so now divide through to get the means
            for (i = 0; i < ff1.connections.size(); ++i) {
                mean_gradients[i].first /= static_cast<float>(mini_batch_size);
                mean_gradients[i].second /= static_cast<float>(mini_batch_size);
            }
            cost /= static_cast<float>(mini_batch_size);
            costfile << cost << std::endl;

            // perform the gradient update. v -> v' = v - eta * gradC
            i = 0;
            for (auto& c : ff1.connections) {
                c.ws[0] -= (mean_gradients[i].first * eta);
                c.b -= (mean_gradients[i].second * eta);
                ++i;
            }
        }

        // Evaluate the latest network at the end of the epoch (we just trained on the 60000 input patterns)
        unsigned int numcorrect = ff1.evaluate (m.test_f);
        std::cout << "In that last Epoch, "<< numcorrect << "/10000 were characterized correctly" << std::endl;
    }

    costfile.close();

    return 0;
}
