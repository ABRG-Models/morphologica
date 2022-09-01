/*
 * Train a small feedforward neural network on the XOR problem.
 *
 * \author Seb James
 * \date Aug 2022
 */

#include <morph/Mnist.h>
#include <morph/Random.h>
#include <morph/nn/FeedForwardNet.h>
#include <morph/vVector.h>
#include <fstream>
#include <vector>
#include <utility>
#include <sstream>

#include <morph/Visual.h>
#include <morph/GraphVisual.h>
#include "netvisual.h"

int main (int argc, char** argv)
{
    // Create a feed-forward network
    std::vector<unsigned int> layer_spec = {2,2,1};
    morph::nn::FeedForwardNet<float> ff1(layer_spec);
    std::cout << ff1 << std::endl;

    // The XOR function has 4 possible inputs and 4 correct outputs.
    std::vector<morph::vVector<float>> ins = {{1, 0}, {0, 1}, {0, 0}, {1, 1}};
    std::vector<morph::vVector<float>> outs = {{1}, {1}, {0}, {0}};

    // main loop, while m.training_f has values in:
    unsigned int epochs = 5000; // Here, an epoch is a run through each batch of in/outs.
    unsigned int mini_batch_size = ins.size();
    float eta = 1.0f;
    std::stringstream ss;
    if (argc > 1) {
        ss << argv[1];
        ss >> eta;
    }
    std::cout << "eta = " << eta << std::endl;
    float cost = 0.0f;

    // Accumulate the dC/dw and dC/db values in gradients. for each pair, the first
    // is nabla_w the second is nabla_b. There are as many pairs as there are
    // connections in ff1.
    std::vector<std::pair<morph::vVector<float>, morph::vVector<float>>> mean_gradients;
    // Init mean gradients
    for (auto& c : ff1.connections) {
        mean_gradients.push_back (std::make_pair(c.nabla_ws[0], c.nabla_b));
    }
    for (auto g : mean_gradients) {
        std::cout << "nabla_w: " << g.first << ", nabla_b: " << g.second << std::endl;
    }

    std::ofstream costfile;
    costfile.open ("cost.csv", std::ios::out|std::ios::trunc);

    morph::vVector<float> costs(epochs, 0.0f);
    morph::vVector<float> epoch_container(epochs, 0);

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
            morph::vVector<float> thein = ins[mb];
            morph::vVector<float> theout = outs[mb];

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

        // Have accumulated cost and mean_gradients, so now divide through to get the means
        for (i = 0; i < ff1.connections.size(); ++i) {
            mean_gradients[i].first /= static_cast<float>(mini_batch_size);
            mean_gradients[i].second /= static_cast<float>(mini_batch_size);
        }
        cost /= (static_cast<float>(mini_batch_size));
        costfile << cost << std::endl;

        // Store for graph
        costs[ep] = cost;
        epoch_container[ep] = static_cast<float>(ep);

        // Gradient update. v -> v' = v - eta * gradC
        i = 0;
        for (auto& c : ff1.connections) {
            c.ws[0] -= (mean_gradients[i].first * eta);
            c.b -= (mean_gradients[i].second * eta);
            ++i;
        }
    }
    costfile.close();

    std::cout << "Evaluate final network:\n================\n";
    ff1.evaluate (ins, outs);
    std::cout << "FINAL NETWORK:\n================\n" << ff1;

    // Visualise the network
    morph::Visual v(1000, 1000, "XOR network");
    v.lightingEffects();
    NetVisual<float>* nv = new NetVisual<float>(v.shaderprog, v.tshaderprog, {0,0,0}, &ff1);
    nv->finalize();
    v.addVisualModel (nv);

    morph::GraphVisual<float>* gv = new morph::GraphVisual<float>(v.shaderprog, v.tshaderprog, {1,0.5,0});
    morph::DatasetStyle ds(morph::stylepolicy::lines);
    ds.linecolour =  {1.0, 0.0, 0.0};
    ds.linewidth = 0.015f;
    gv->xlabel = "Epoch";
    gv->ylabel = "Cost";
    gv->setdata (epoch_container, costs, ds);
    gv->finalize();
    v.addVisualModel (gv);

    v.keepOpen();

    return 0;
}
