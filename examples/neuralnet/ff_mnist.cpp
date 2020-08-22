/*
 * Train a neural network to characterise the MNIST database of numerals.
 *
 * \author Seb James
 * \date May-Aug 2020
 */

#include "Mnist2.h"
#include <fstream>
#include "FeedForward.h"
#include <random>
#include <algorithm>
#include <chrono>
using namespace std::chrono;

int main()
{
    // Read the MNIST data
    Mnist2 m;

    // Instantiate the network
    FeedForwardNet<float> ff1({784,32,10}, &(m.training_f.front()));

    // main loop parameters are number of epochs, the size of a mini-batch and the
    // learning rate eta
    unsigned int epochs = 10;
    unsigned int mini_batch_size = 10;
    float eta = 3.0f;

    // Accumulate the dC/dw and dC/db values in gradients. for each pair, the first is
    // nabla_w the second is nabla_b. There are as many pairs as there are connections
    // in ff1. Here, we declare an initialize mean_gradients
    std::vector<std::pair<morph::vVector<float>, morph::vVector<float>>> mean_gradients;
    for (auto& c : ff1.connections) {
        mean_gradients.push_back (std::make_pair(c.nabla_w, c.nabla_b));
    }

    // i is used as a short term iterator variable for several loops
    unsigned int i = 0;

    std::vector<unsigned int> idxOrdered (m.training_f.size(), 0);
    for (unsigned int idx = 0; idx < idxOrdered.size(); ++idx) { idxOrdered[idx] = idx; }

    // For the shuffle
    std::random_device rd;
    std::mt19937 g(rd());

    // Desired output
    morph::vVector<float> theout(10);

    for (unsigned int ep = 0; ep < epochs; ++ep) {

        std::cout << "Epoch " << ep << "...\n";
        milliseconds ff_time = std::chrono::milliseconds::zero();
        milliseconds cc_time = std::chrono::milliseconds::zero();
        milliseconds bp_time = std::chrono::milliseconds::zero();
        milliseconds ln_time = std::chrono::milliseconds::zero();

        // Shuffle index order
        std::shuffle (idxOrdered.begin(), idxOrdered.end(), g);

        // Index into idxOrdered, m.training_f, m.training_label.
        unsigned int idx_seq = 0;
        unsigned int idx = 0;

        for (unsigned int j = 0; j < m.training_f.size()/mini_batch_size; ++j) {

            // Zero the mean gradents
            for (i = 0; i < ff1.connections.size(); ++i) {
                mean_gradients[i].first.zero();
                mean_gradients[i].second.zero();
            }

            // Loop through each member of the mini-batch
            for (unsigned int mb = 0; mb < mini_batch_size; ++mb) {

                // From the sequential index, get the randomised index.
                idx = idxOrdered[idx_seq++];

                // Set up input
                theout.zero();
                theout[static_cast<size_t>(m.training_label[idx])] = 1.0f;

                ff1.setInput (&m.training_f[idx], theout);

                // Feedforward, then back-propagate errors
                milliseconds ms1 = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
                ff1.feedforward();
                milliseconds ms1_2 = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
                ff1.computeCost();
                milliseconds ms2 = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
                ff_time += (ms1_2-ms1);
                cc_time += (ms2-ms1_2);
                milliseconds ms3 = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
                ff1.backprop();
                milliseconds ms4 = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
                bp_time += (ms4-ms3);

                // Now collect up the nabla_w and nabla_bs for the learning step (already summed up cost)
                i = 0;
                for (auto& c : ff1.connections) {
                    mean_gradients[i].first += c.nabla_w;
                    mean_gradients[i].second += c.nabla_b;
                    ++i;
                }
                milliseconds ms5 = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
                ln_time += (ms5-ms4);
            }

            // Have accumulated cost and mean_gradients, so now divide through to get the means
            for (i = 0; i < ff1.connections.size(); ++i) {
                mean_gradients[i].first /= static_cast<float>(mini_batch_size);
                mean_gradients[i].second /= static_cast<float>(mini_batch_size);
            }

            // perform the gradient update. v -> v' = v - eta * gradC
            i = 0;
            for (auto& c : ff1.connections) {
                c.w -= (mean_gradients[i].first * eta);
                c.b -= (mean_gradients[i].second * eta);
                ++i;
            }
        }

        // Evaluate the latest network at the end of the epoch (we just trained on the 60000 input patterns)
        unsigned int numcorrect = ff1.evaluate (m.test_f, m.test_label);
        std::cout << "In that last Epoch, "<< numcorrect << "/10000 were characterized correctly" << std::endl;
        std::cout << "FF/CC/BP/LN times: " << ff_time.count() << "/" << cc_time.count() << "/" << bp_time.count() << "/" << ln_time.count() << "\n";
    }

    return 0;
}
