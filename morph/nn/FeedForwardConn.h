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
#include <vector>

namespace morph {
    namespace nn {

        /*!
         * A connection between neuron layers in a feed forward neural network. This
         * connects any number of input neuron populations to a single output
         * population.
         */
        template <typename T>
        struct FeedForwardConn
        {
            //! Construct for connection from single input layer to single output layer
            FeedForwardConn (morph::vVector<T>* _in, morph::vVector<T>* _out)
            {
                this->ins.resize(1);
                this->ins[0] = _in;
                this->M = _in->size();

                this->commonInit (_out);
            }

            //! Construct for connection from many input layers to single output layer
            FeedForwardConn (std::vector<morph::vVector<T>*> _ins, morph::vVector<T>* _out)
            {
                this->ins = _ins;
                this->M = 0;
                for (auto _in : this->ins) { this->M += _in->size(); }

                this->commonInit(_out);
            }

            //! Init common to all constructors
            void commonInit (morph::vVector<T>* _out)
            {
                this->out = _out;
                this->N = this->out->size();
                //this->delta.resize (M, T{0});
                this->deltas.resize (this->ins.size());
                for (unsigned int i = 0; i < this->deltas.size(); ++i) {
                    this->deltas[i].resize (this->ins[i]->size(), T{0});
                }
                //this->w.resize (M*N, T{0});
                this->ws.resize (this->ins.size());
                for (unsigned int i = 0; i < this->ws.size(); ++i) {
                    this->ws[i].resize(this->ins[i]->size() * this->N, T{0});
                }
                //this->nabla_w.resize (M*N, T{0});
                this->nabla_ws.resize (this->ins.size());
                for (unsigned int i = 0; i < this->nabla_ws.size(); ++i) {
                    this->nabla_ws[i].resize(this->ins[i]->size() * this->N, T{0});
                    this->nabla_ws[i].zero();
                }
                //this->nabla_w.zero();
                this->b.resize (N, T{0});
                this->nabla_b.resize (N, T{0});
                this->nabla_b.zero();
                this->z.resize (N, T{0});
                this->z.zero();
            }

            //! Input layer has size M.
            std::vector<morph::vVector<T>*> ins;
            size_t M = 0;
            //! Pointer to output layer. Size N.
            morph::vVector<T>* out;
            size_t N = 0;
            //! The errors in the input layer of neurons. Size M = m1 + m2 +...
            std::vector<morph::vVector<T>> deltas;
            //! Weights.
            //! Order of weights: w_11, w_12,.., w_1M, w_21, w_22, w_2M, etc. Size M by N = m1xN + m2xN +...
            std::vector<morph::vVector<T>> ws;
            //! Biases. Size N.
            morph::vVector<T> b;
            //! The gradients of cost vs. weights. Size M by N = m1xN + m2xN +...
            std::vector<morph::vVector<T>> nabla_ws;
            //! The gradients of cost vs. biases. Size N.
            morph::vVector<T> nabla_b;
            //! Activation of the output neurons. Computed in feedforward, used in backprop
            //! z = sum(w.in) + b. Final output written into *out is the sigmoid(z). Size N.
            morph::vVector<T> z;

            //! Output as a string
            std::string str() const
            {
                std::stringstream ss;
                for (auto w : this->ws) {
                    ss << "Weights: w" << w << "w (" << w.size() << ")\n";
                }
                for (auto nabla_w : this->nabla_ws) {
                    ss << "nabla_w:nw" << nabla_w << "nw (" << nabla_w.size() << ")\n";
                }
                ss << " Biases: b" << b << "b (" << b.size() << ")\n";
                ss << "nabla_b:nb" << nabla_b << "nb (" << nabla_b.size() << ")\n";
                for (auto delta : this->deltas) {
                    ss << "delta  :  " << delta << "\n";
                }
                return ss.str();
            }

            //! Randomize the weights and biases
            void randomize()
            {
                for (auto& w : this->ws) { w.randomizeN (T{0.0}, T{1.0}); }
                this->b.randomizeN (T{0.0}, T{1.0});
            }

            // Will need to split out the sigmoid here into a separate function. Can then do
            // feedforward from a number of input layers, with a final sigmoid. Hmm. No. Because
            // who then owns the bias? Need to include all the inputs as I originally thought.
            //
            //! Feed-forward compute. out[i] = in[0,..,M-1] . w[i,..,i+M-1] + b[i]
            void feedforward()
            {
                // First, for each output, set the activation, z to 0
                for (size_t j = 0; j < this->N; ++j) { this->z[j] = 0; }

                // For each input population:
                for (size_t i = 0; i < this->ins.size(); ++i) {
                    // A morph::vVector for a 'part of w'
                    size_t m = this->ins[i]->size();// Size m[i]
                    morph::vVector<T> wpart(m);
                    // Get weights, outputs and biases iterators
                    auto witer = this->ws[i].begin();
                    // Carry out an N sized for loop computing each output
                    for (size_t j = 0; j < this->N; ++j) { // Each output
                        // Copy part of weights to wpart (M elements):
                        std::copy (witer, witer+m, wpart.begin());
                        // Compute/accumulate dot product with input
                        this->z[j] += wpart.dot (*ins[i]);
                        // Move to the next part of the weight matrix for the next loop
                        witer += m;
                    }
                }

                // Finally, for each output, apply the transfer function
                this->applyTransfer();
            }

            //! For each output, add bias and apply transfer
            void applyTransfer()
            {
                auto oiter = this->out->begin();
                auto biter = this->b.begin();
                for (size_t j = 0; j < this->N; ++j) {
                    this->z[j] += *biter++;
                    *oiter++ = T{1} / (T{1} + std::exp(-z[j])); // out = sigmoid(z)
                }
            }

            //! The content of *FeedForwardConn::out is sigmoid(z^l+1). \return has size N
            morph::vVector<T> sigmoid_prime_z_lplus1() { return (*out) * (-(*out)+T{1}); }

            //! The content of *FeedForwardConn::in is sigmoid(z^l). \return has size M = m1 + m2 +...
            std::vector<morph::vVector<T>> sigmoid_prime_z_l()
            {
                std::vector<morph::vVector<T>> rtn (this->ins.size());
                for (size_t i = 0; i < this->ins.size(); ++i) {
                    rtn[i] = (*ins[i]) * (-(*ins[i])+T{1});
                }
                return rtn;
            }

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
                std::vector<morph::vVector<T>> w_times_deltas(this->ins.size());
                for (size_t idx = 0; idx < this->ins.size(); ++idx) {
                    size_t m = this->ins[idx]->size();
                    w_times_deltas[idx].resize(m);
                    w_times_deltas[idx].zero();
                    for (size_t i = 0; i < m; ++i) { // Each input
                        for (size_t j = 0; j < this->N; ++j) { // Each output
                            // For each weight fanning into neuron j in l_nxt, sum up:
                            w_times_deltas[idx][i] += this->ws[idx][i+(m*j)] * delta_l_nxt[j];
                        }
                    }
                }

                std::vector<morph::vVector<T>> spzl = this->sigmoid_prime_z_l(); // spzl has size M; deriv of input

                if (spzl.size() < this->deltas.size()) {
                    throw std::runtime_error ("Sizes error (spzl and deltas)");
                }

                for (size_t idx = 0; idx < this->deltas.size(); ++idx) {
                    this->deltas[idx] = w_times_deltas[idx] * spzl[idx];
                }

                // NB: In a given connection, we compute nabla_b and nabla_w relating to the
                // *output* neurons and the weights also related to the output neurons.

                this->nabla_b = delta_l_nxt; // Size is N

                for (size_t idx = 0; idx < this->ins.size(); ++idx) {
                    size_t m = this->ins[idx]->size();
                    for (size_t i = 0; i < m; ++i) { // Each input
                        for (size_t j = 0; j < this->N; ++j) { // Each output
                            // nabla_w is a_in * delta_out:
                            this->nabla_ws[idx][i+(m*j)] = (*(ins[idx]))[i] * delta_l_nxt[j];
                        }
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
