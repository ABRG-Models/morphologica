/*
 * Train a small feedforward neural network on the XOR problem.
 *
 * \author Seb James
 * \date Aug 2022
 */

#include <morph/Mnist.h>
#include <morph/Random.h>
#include <morph/nn/FeedForwardNet.h>
#include <morph/vvec.h>
#include <fstream>
#include <vector>
#include <utility>
#include <sstream>

#include <morph/Visual.h>
#include <morph/GraphVisual.h>
#include "netvisual.h"

//! Helper function to save PNG images
void savePngs (const std::string& logpath, const std::string& name,
               unsigned int frameN, morph::Visual& v) {
    std::stringstream ss;
    ss << logpath << "/" << name<< "_";
    ss << std::setw(5) << std::setfill('0') << frameN;
    ss << ".png";
    v.saveImage (ss.str());
}

int main (int argc, char** argv)
{
    // Create a feed-forward network. layer_spec defines the shape of the network.
    std::vector<unsigned int> layer_spec = {2,2,1};
    morph::nn::FeedForwardNet<float> ff1(layer_spec);

    // The XOR function has 4 possible inputs and 4 correct outputs.
    std::vector<morph::vvec<float>> ins = {{1, 0}, {0, 1}, {0, 0}, {1, 1}};
    std::vector<morph::vvec<float>> outs = {{1}, {1}, {0}, {0}};

    // main loop, while m.training_f has values in:
    unsigned int epochs = 5000; // Here, an 'epoch' is a run through each batch of in/outs.
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
    std::vector<std::pair<morph::vvec<float>, morph::vvec<float>>> mean_gradients;
    // Init mean gradients
    for (auto& c : ff1.connections) {
        mean_gradients.push_back (std::make_pair(c.nabla_ws[0], c.nabla_b));
    }
    for (auto g : mean_gradients) {
        std::cout << "nabla_w: " << g.first << ", nabla_b: " << g.second << std::endl;
    }

    // Visualise the network during operation
    morph::Visual v(1920, 1080, "XOR network");
    v.setSceneTrans (-0.738625824,-0.950026929,-3.00000191);
    v.lightingEffects();
    NetVisual<float>* nv = new NetVisual<float>(v.shaderprog, v.tshaderprog, {0,0,0}, &ff1);
    nv->finalize(); // Generates the model before adding it to the Visual scene
    v.addVisualModel (nv);

    // Create a graph to visualise cost vs. epoch
    morph::GraphVisual<float>* gv = new morph::GraphVisual<float>(v.shaderprog, v.tshaderprog, {1,0.5,0});
    morph::DatasetStyle ds(morph::stylepolicy::lines);
    ds.linecolour =  {1.0, 0.0, 0.0};
    ds.linewidth = 0.015f;
    gv->xlabel = "Epoch";
    gv->ylabel = "Cost";
    gv->setlimits (0, 5000, 0, 0.25); // Sets axes limits
    gv->prepdata (ds); // prepares data which will be appended to with gv->append()
    gv->finalize();
    v.addVisualModel (gv);

    // Used throughout loop as an iterator variable
    unsigned int i = 0;
    // Used to number saved frames for a movie
    unsigned int framenum = 0;

    for (unsigned int ep = 0; ep < epochs; ++ep) {

        // Zero the mean gradients for each epoch
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

        // Have accumulated cost and mean_gradients, so now divide through to get the means
        for (i = 0; i < ff1.connections.size(); ++i) {
            mean_gradients[i].first /= static_cast<float>(mini_batch_size);
            mean_gradients[i].second /= static_cast<float>(mini_batch_size);
        }
        cost /= (static_cast<float>(mini_batch_size));

        // Gradient update. v -> v' = v - eta * gradC
        i = 0;
        for (auto& c : ff1.connections) {
            c.ws[0] -= (mean_gradients[i].first * eta);
            c.b -= (mean_gradients[i].second * eta);
            ++i;
        }

         // Update graph. Append the cost value to the dynamic graph
        gv->append (ep, cost, 0);

        // Update network visualization
        nv->clear(); // Call clear to remove texts from the netvisual
        nv->reinit(); // Rebuild the netvisual, using the new values for acts, weights and biases

        // Render and save a picture for a movie
        if (ep%20 == 0) {
            glfwWaitEventsTimeout (0.01);
            v.render();
            savePngs ("./logs", "ff_xor", framenum++, v);
        }
    }

    std::cout << "Evaluate final network:\n================\n";
    ff1.evaluate (ins, outs);
    // You can output the network to stdout like this:
    std::cout << "FINAL NETWORK:\n================\n" << ff1;

    v.keepOpen(); // Keeps graphics window open until user closes it.

    return 0;
}
