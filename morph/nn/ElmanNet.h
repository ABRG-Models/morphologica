/*!
 * \file
 *
 * This file contains an Elman neural network class, whose neuron layer sizes can be
 * configured at runtime.
 *
 * \author Seb James
 * \date October 2020
 */
#pragma once

#include <ostream>
#include <vector>
#include <list>
#include <sstream>
#include <iostream>
#include <morph/vVector.h>
#include <morph/nn/FeedForwardConn.h>

namespace morph {
    namespace nn {

        /*!
         * An Elman network, in which an input layer feeds forward to a hidden layer,
         * which feeds forward to an output layer, and also back to a context (or
         * memory) layer, which then feeds forward to the hidden layer.
         */
        template <typename T>
        struct ElmanNet
        {
            //! Constructor takes a vector specifying the number of neurons in each layer (\a
            //! layer_spec)
            ElmanNet (const std::vector<unsigned int>& layer_spec)
            {
                unsigned int n_layers = layer_spec.size();
                // Set up initial conditions
                for (unsigned int nn = 0; nn < n_layers; ++nn) {
                    // Create, and zero, a layer containing nn neurons:
                    morph::vVector<T> lyr(layer_spec[nn]);
                    lyr.zero();

                    // Add the layer to this->neurons
                    this->neurons.push_back (lyr);

                    // If we're on a hidden layer, then also create a context layer
                    if (nn > 0 && nn < (n_layers-1)) {
                        this->contextNeurons.push_back (lyr);
                    }

                    // If there was a 'lastLayer', then add a connection between the layers
                    if (nn > 0) {
                        auto l = this->neurons.end();
                        --l;
                        auto lm1 = l;
                        --lm1;

                        //std::cout << "New connection " << nn << std::endl;
                        // Combine two inputs if this is not a connection to the output layer
                        std::vector<morph::vVector<T>*> _inputs;
                        _inputs.push_back (&*lm1); // The neuron 'layer minus one'
                        //std::cout << " with input " << *lm1 << std::endl;
                        if (nn < (n_layers-1)) {
                            auto cl = this->contextNeurons.end();
                            --cl;
                            _inputs.push_back (&*cl); // The context layer
                            //std::cout << " AND input " << *cl << std::endl;
                        } // just the one input (for the final layer)

                        morph::nn::FeedForwardConn<T> c(_inputs, &*l);
                        c.randomize();

                        this->connections.push_back (c);
                    }
                }
            }

            //! Output the network as a string
            std::string str() const
            {
                std::stringstream ss;
                unsigned int n_max = this->neurons.size();
                auto c = this->connections.begin();
                auto n = this->neurons.begin();
                auto cn = this->contextNeurons.begin();
                for (unsigned int i = 0; i < n_max; ++i) {
                    if (i>0) { ss << *c++; }
                    ss << "Layer " << i << " neurons:  "  << *n++ << "\n";
                    if (i>0 && i<(n_max-1)) {
                        ss << "Layer " << i << " context:  "  << *cn++ << "\n";
                    }
                }
                ss << "Network target out: " << this->desiredOutput << "\n";
                ss << "Network delta_out:  " << this->delta_out << "\n";
                ss << "Network cost:       " << this->cost << "\n";
                return ss.str();
            }

            //! Update the network's outputs from its inputs
            void feedforward()
            {
                // Step 1, feed back the context results from the last run into the contextNeuron layers
                typename std::list<morph::vVector<T>>::iterator cni = contextNeurons.begin();
                typename std::list<morph::vVector<T>>::iterator ni = neurons.begin();
                ++ni; // skip first neuron layer
                while (cni != contextNeurons.end()) {
                    if (ni == neurons.end()) { throw std::runtime_error ("Not enough neuron layers"); }
                    // At t+1, the context units contain values which are exactly the
                    // hidden unit values at time t. Thus, we simply copy here
                    std::copy (ni->begin(), ni->end(), cni->begin());
                    ++cni; ++ni;
                }

                // Step 2, feed forward as normal
                auto c = this->connections.begin();
                for (size_t i = 0; i < this->connections.size(); ++i) {
                    c->feedforward();
                    c++;
                }
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
                    std::cout << "Input " << ir << " --> " << this->neurons.back()
                              << " cf. " << this->desiredOutput << " (cost: " << c << ")\n";
                }
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
                    // Now citer is closer to input.
                    citer->backprop (*citer_closertooutput);
                }
            }

            //! Set up an input along with desired output
            void setInput (const morph::vVector<T>& theInput, const morph::vVector<T>& theOutput)
            {
                *(this->neurons.begin()) = theInput;
                // Set each context layer to 0.5, initially
                for (auto& cl : this->contextNeurons) {
                    cl.set_from (T{0.5});
                }
                this->desiredOutput = theOutput;
            }

            //! Compute the cost (and delta_out) for the current input and desired output
            T computeCost()
            {
                // Here is where we compute delta_out:
                this->delta_out = (this->neurons.back()-desiredOutput) * (this->connections.back().sigmoid_prime_z_lplus1());
                // And the cost:
                morph::vVector<T> prediction = this->neurons.back();
                // Want the prediction error, which is either correct or incorrect.
                for (auto& p : prediction) {
                    p = p > T{0.5} ? T{1} : T{0};
                }
                T e = (desiredOutput-prediction).length();
                // Elman seems to use '0.5 * binary error squared' for the cost:
                this->cost = morph::nn::ElmanNet<T>::costKernel (e);
                return this->cost;
            }

            //! A static cost kernel to allow external code to compute an error using the same method as ElmantNet::computeCost.
            static T costKernel (T& binary_error) { return T{0.5} * binary_error * binary_error; }

            //! What's the cost function of the current output? Computed in computeCost()
            T cost = T{0};

            //! A variable number of neuron layers, each of variable size. NB: get
            //! memory errors if this is an std::vector of morph::vVectors. Don't know why.
            std::list<morph::vVector<T>> neurons;

            //! Context neuron layers - those that are fed back to. This size of this vector is
            //! neurons.size()-2 - that is there is one context layer for each hidden layer in
            //! the feedforward stack.
            std::list<morph::vVector<T>> contextNeurons;

            //! Connections. There should be neurons.size()-1 connection layers:
            std::list<morph::nn::FeedForwardConn<T>> connections;

            //! The error (dC/dz) of the output layer
            morph::vVector<T> delta_out;

            //! The desired output of the network
            morph::vVector<T> desiredOutput;
        };

        template <typename T>
        std::ostream& operator<< (std::ostream& os, const morph::nn::ElmanNet<T>& el)
        {
            os << el.str();
            return os;
        }
    } // namespace nn
} // namespace morph
