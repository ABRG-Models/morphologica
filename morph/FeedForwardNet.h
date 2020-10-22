/*!
 * \file
 *
 * This file contains a feed-forward neural network class, whose neuron
 * sizes can be configured at runtime.
 *
 * \author Seb James
 * \date May 2020
 */
#pragma once

#include <morph/FeedForward.h>
#include <morph/vVector.h>
#include <iostream>
#include <list>
#include <vector>
#include <sstream>
#include <ostream>
#include <map>

namespace morph {
    namespace nn {

        /*!
         * A feedforward network class which holds a runtime-selectable set of neuron
         * layers and the connections between the layers. Note that in this class, the
         * connections are always between adjacent layers; from layer l to layer l+1.
         */
        template <typename T>
        struct FeedForwardNet
        {
            //! Constructor takes a vector specifying the number of neurons in each layer (\a
            //! layer_spec)
            FeedForwardNet (const std::vector<unsigned int>& layer_spec)
            {
                // Set up initial conditions
                for (auto nn : layer_spec) {
                    // Create, and zero, a layer containing nn neurons:
                    morph::vVector<T> lyr(nn);
                    lyr.zero();
                    size_t lastLayerSize = 0;
                    if (!this->neurons.empty()) { // Set lastLayerSize
                        lastLayerSize = this->neurons.back().size();
                    }
                    // Add the layer to this->neurons
                    this->neurons.push_back (lyr);
                    // If there was a 'lastLayer', then add a connection between the layers
                    if (lastLayerSize != 0) {
                        auto l = this->neurons.end();
                        --l;
                        auto lm1 = l;
                        --lm1;
                        morph::nn::FeedForwardConn<T> c(&*lm1, &*l);
                        c.randomize();
                        this->connections.push_back (c);
                    }
                }
            }

            //! Output the network as a string
            std::string str() const
            {
                std::stringstream ss;
                unsigned int i = 0;
                auto c = this->connections.begin();
                for (auto n : this->neurons) {
                    if (i>0 && c != this->connections.end()) {
                        ss << *c++;
                    }
                    ss << "Layer " << i++ << ":  "  << n << "\n";
                }
                ss << "Target output: " << this->desiredOutput << "\n";
                ss << "Delta out: " << this->delta_out << "\n";
                ss << "Cost:      " << this->cost << "\n";
                return ss.str();
            }

            //! Update the network's outputs from its inputs
            void feedforward()
            {
                for (auto& c : this->connections) { c.feedforward(); }
            }

            //! A function which shows the difference between the network output and
            //! desiredOutput for debugging
            void evaluate (const std::vector<morph::vVector<float>>& ins,
                           const std::vector<morph::vVector<float>>& outs)
            {
                auto op = outs.begin();
                for (auto ir : ins) {
                    // Set input and output
                    this->neurons.front() = ir;
                    this->desiredOutput = *op++;
                    // Compute network and cost
                    this->feedforward();
                    float c = this->computeCost();
                    std::cout << "Input " << ir << " --> " << this->neurons.back() << " cf. " << this->desiredOutput << " (cost: " << c << ")\n";
                }
            }

            // FIXME: Put in a derived class specifically for Mnist handling.
            //! Evaluate against the Mnist test image set
            unsigned int evaluate (const std::multimap<unsigned char, morph::vVector<float>>& testData, int num=10000)
            {
                // For each image in testData, compute cost
                float evalcost = 0.0f;
                unsigned int numMatches = 0;
                int count = 0;
                for (auto img : testData) {
                    unsigned int key = static_cast<unsigned int>(img.first);
                    // Set input
                    this->neurons.front() = img.second;
                    // Set output
                    this->desiredOutput.zero();
                    this->desiredOutput[key] = 1.0f;
                    // Update
                    this->feedforward();
                    evalcost += this->computeCost();
                    // Success?
                    if (this->neurons.back().argmax() == key) {
                        ++numMatches;
                    }
                    ++count;
                    if (count >= num) {
                        break;
                    }
                }
                return numMatches;
            }

            //! Determine the error gradients by the backpropagation method. NB: Call
            //! computeCost() first
            void backprop()
            {
                // Notation follows http://neuralnetworksanddeeplearning.com/chap2.html
                // The output layer is special, as the error in the output layer is given by
                //
                // delta^L = grad_a(C) 0 sigma_prime(z^L)
                //
                // whereas for the intermediate layers
                //
                // delta^l = w^l+1 . delta^l+1 0 sigma_prime (z^l)
                //
                // (where 0 signifies hadamard product, as implemented by vVector's operator*)
                // delta = dC_x/da() * sigmoid_prime(z_out)
                auto citer = this->connections.end();
                --citer; // Now points at output layer
                citer->backprop (this->delta_out); // Layer L delta computed
                // After the output layer, loop through the rest of the layers:
                for (;citer != this->connections.begin();) {
                    auto citer_closertooutput = citer--;
                    // Now citer is closer to input
                    citer->backprop (citer_closertooutput->delta);
                }
            }

            //! Set up an input along with desired output
            void setInput (const morph::vVector<T>& theInput, const morph::vVector<T>& theOutput)
            {
                *(this->neurons.begin()) = theInput;
                this->desiredOutput = theOutput;
            }

            //! Compute the cost for one input and one desired output
            T computeCost()
            {
                // Here is where we compute delta_out:
                this->delta_out = (this->neurons.back()-desiredOutput) * (this->connections.back().sigmoid_prime_z_lplus1());
                // And the cost:
                T l = (desiredOutput-this->neurons.back()).length();
                this->cost = T{0.5} * l * l;
                return this->cost;
            }

            //! What's the cost function of the current output? Computed in computeCost()
            T cost = T{0};

            //! A variable number of neuron layers, each of variable size.
            std::list<morph::vVector<T>> neurons;
            //! Connections. There should be neurons.size()-1 connection layers:
            std::list<morph::nn::FeedForwardConn<T>> connections;
            //! The error (dC/dz) of the output layer
            morph::vVector<T> delta_out;
            //! The desired output of the network
            morph::vVector<T> desiredOutput;
        };

        template <typename T>
        std::ostream& operator<< (std::ostream& os, const morph::nn::FeedForwardNet<T>& ff)
        {
            os << ff.str();
            return os;
        }
    } // namespace nn
} // namespace morph
