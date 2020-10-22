/*!
 * \file
 *
 * This file contains a class to hold the information about the connections between
 * layers of neurons in the network.
 *
 * \author Seb James
 * \date May 2020
 */
#pragma once

#include <morph/vVector.h>
#include <iostream>
#include <sstream>
#include <ostream>

namespace morph {
    namespace nn {

        /*!
         * A connection between neuron layers in a simple, stacked neural network. This
         * connects a number of input neuron populations to a single output population.
         */
        template <typename T>
        struct FeedForwardConn
        {
            FeedForwardConn (morph::vVector<T>* _in, morph::vVector<T>* _out)
            {
                this->in = _in;
                this->out = _out;
                this->M = this->in->size();
                this->N = this->out->size();
                this->delta.resize (M, T{0});
                this->w.resize (M*N, T{0});
                this->nabla_w.resize (M*N, T{0});
                this->nabla_w.zero();
                this->b.resize (N, T{0});
                this->nabla_b.resize (N, T{0});
                this->nabla_b.zero();
                this->z.resize (N, T{0});
                this->z.zero();
            }

            //! Input layer has size M.
            morph::vVector<T>* in;
            size_t M = 0;
            //! Pointer to output layer. Size N.
            morph::vVector<T>* out;
            size_t N = 0;
            //! The errors in the input layer of neurons. Size M.
            morph::vVector<T> delta;
            //! Weights.
            //! Order of weights: w_11, w_12,.., w_1M, w_21, w_22, w_2M, etc. Size M by N.
            morph::vVector<T> w;
            //! Biases. Size N.
            morph::vVector<T> b;
            //! The gradients of cost vs. weights. Size M by N.
            morph::vVector<T> nabla_w;
            //! The gradients of cost vs. biases. Size N.
            morph::vVector<T> nabla_b;
            //! Activation of the output neurons. Computed in feedforward, used in backprop
            //! z = sum(w.in) + b. Final output written into *out is the sigmoid(z). Size N.
            morph::vVector<T> z;

            //! Output as a string
            std::string str() const
            {
                std::stringstream ss;
                ss << "Weights: w" << w << "w (" << w.size() << ")\n";
                ss << "nabla_w:nw" << nabla_w << "nw (" << nabla_w.size() << ")\n";
                ss << " Biases: b" << b << "b (" << b.size() << ")\n";
                ss << "nabla_b:nb" << nabla_b << "nb (" << nabla_b.size() << ")\n";
                ss << "delta  :  " << delta << "\n";
                return ss.str();
            }

            //! Randomize the weights and biases
            void randomize()
            {
                this->w.randomizeN (T{0.0}, T{1.0});
                this->b.randomizeN (T{0.0}, T{1.0});
            }

            // Will need to split out the sigmoid here into a separate function. Can then do
            // feedforward from a number of input layers, with a final sigmoid. Hmm. No. Because
            // who then owns the bias? Need to include all the inputs as I originally thought.
            //
            //! Feed-forward compute. out[i] = in[0,..,M-1] . w[i,..,i+M-1] + b[i]
            void feedforward()
            {
                // A morph::vVector for a 'part of w'
                morph::vVector<T> wpart(this->in->size()); // Size M
                // Get weights, outputs and biases iterators
                auto witer = this->w.begin();
                auto oiter = this->out->begin();
                auto biter = this->b.begin();
                // Carry out an N sized for loop computing each output
                for (size_t j = 0; j < this->N; ++j) { // Each output
                    // Copy part of weights to wpart (M elements):
                    std::copy (witer, witer+this->M, wpart.begin());
                    // Compute dot product with input and add bias:
                    this->z[j] += wpart.dot (*in) + *biter++;
                    std::cout << "z[j=" << j << "]=" << this->z[j] << std::endl;
                    *oiter++ = T{1} / (T{1} + std::exp(-z[j])); // out = sigmoid(z)
                    // Move to the next part of the weight matrix for the next loop
                    witer += this->M;
                }
            }

            void applyTransfer()
            {
                auto oiter = this->out->begin();
                for (size_t j = 0; j < this->N; ++j) { // Each output
                    *oiter++ = T{1} / (T{1} + std::exp(-z[j])); // out = sigmoid(z)
                }
            }

            //! The content of *FeedForwardConn::out is sigmoid(z^l+1). \return has size N
            morph::vVector<T> sigmoid_prime_z_lplus1() { return (*out) * (-(*out)+T{1}); }

            //! The content of *FeedForwardConn::in is sigmoid(z^l). \return has size M
            morph::vVector<T> sigmoid_prime_z_l() { return (*in) * (-(*in)+T{1}); }

            /*!
             * Compute this->delta using the values computed in FeedForwardConn::feedforward
             * (which must have been executed beforehand).
             */
            void backprop (const morph::vVector<T>& delta_l_nxt) // delta_l_nxt has size N.
            {
                if (delta_l_nxt.size() != this->out->size()) {
                    throw std::runtime_error ("backprop: Mismatched size");
                }
                // we have to do weights * delta_l_nxt to give a morph::vVector<T>
                // result. This is the equivalent of the matrix multiplication:
                morph::vVector<T> w_times_delta(this->in->size());
                w_times_delta.zero();
                for (size_t i = 0; i < this->M; ++i) { // Each input
                    for (size_t j = 0; j < this->N; ++j) { // Each output
                        // For each weight fanning into neuron j in l_nxt, sum up:
                        w_times_delta[i] += this->w[i+(this->M*j)] * delta_l_nxt[j];
                    }
                }
                morph::vVector<T> spzl = this->sigmoid_prime_z_l(); // spzl has size M; deriv of input
                this->delta = w_times_delta * spzl;

                // NB: In a given connection, we compute nabla_b and nabla_w relating to the
                // *output* neurons and the weights also related to the output neurons.
                this->nabla_b = delta_l_nxt; // Size is N
                for (size_t i = 0; i < this->M; ++i) { // Each input
                    for (size_t j = 0; j < this->N; ++j) { // Each output
                        // nabla_w is a_in * delta_out:
                        this->nabla_w[i+(this->M*j)] = (*in)[i] * delta_l_nxt[j];
                    }
                }
            }
        };

        //! Stream operator
        template <typename T>
        std::ostream& operator<< (std::ostream& os, const FeedForwardConn<T>& c)
        {
            os << c.str();
            return os;
        }

    } // namespace nn
} // namespace morph
