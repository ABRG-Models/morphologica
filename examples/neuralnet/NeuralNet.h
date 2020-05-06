/*!
 * \file
 *
 * A toy neural net, for a NN example
 *
 * This one follows neuralnetworksanddeeplearning.com
 */

#include <morph/Vector.h>
#include <iostream>

#ifndef MID_LAYER
# define MID_LAYER 30
#endif

//! A connection between a source layer of size M and a destination layer of size N
template <typename T, size_t M, size_t N, size_t MN=M*N>
struct Connection
{
    morph::Vector<T, M>* in;  // Pointer to input layer
    morph::Vector<T, N>* out; // Pointer to output layer
    morph::Vector<T, M> delta; // The errors in the input layer of neurons

    // Order of weights: in[0] to out[all], in[1] to out[all], in[2] to out[all] etc.
    morph::Vector<T, MN> w;
    morph::Vector<T, N> b;
    morph::Vector<T, MN> nabla_w;
    morph::Vector<T, N> nabla_b;

    // Activation of the output neurons. Computed in feedforward, used in backprop
    // z = sum(w.in) + b.
    morph::Vector<T, N> z;

    void randomize() {
        this->w.randomize();
        this->b.randomize();
    }

    //! out[i] = in[0,..,M] . w[i,..,i+M] + b[i]
    void compute() {
        // A morph::Vector for a 'part of w'
        morph::Vector<T, M> wpart;
        // Get weights, outputs and biases iterators
        auto witer = this->w.begin();
        auto oiter = this->out->begin();
        auto biter = this->b.begin();
        // Carry out an N sized for loop computing each output
        for (size_t j = 0; j < N; ++j) {
            // Copy part of weights to wpart:
            std::copy (witer, witer+M, wpart.begin());
            // Compute dot product with input and add bias:
            z[j] = wpart.dot (*in) + *biter++;
            //sigz[j] = T{1} / (T{1} + std::exp(-z[j]));
            //*oiter++ = sigz[j];
            *oiter++ = T{1} / (T{1} + std::exp(-z[j]));
        }
    }

    //! The content of Connection::out is sigmoid(z^l+1)
    morph::Vector<T, N> sigmoid_prime_z_lplus1() {
        return out->hadamard (-(*out)+T{1});
    }

    //! The content of Connection::in is sigmoid(z^l)
    morph::Vector<T, M> sigmoid_prime_z_l() {
        return in->hadamard (-(*in)+T{1});
    }

    //! Compute this->delta using values computed in Connection::compute.
    void backprop (const morph::Vector<T, N>& delta_l_nxt) {
        // we have to do weights * delta_l_nxt to give a morph::Vector<T, N> result. This is
        // the equivalent of the matrix multiplication:
        morph::Vector<T, M> w_times_delta;
        w_times_delta.zero();
        // Should be able to parallelize this:
        for (size_t j = 0; j < N; ++j) {
            for (size_t i = 0; i < M; ++i) {
                w_times_delta[i] +=  this->w[i+(M*j)] * delta_l_nxt[j];
            }
        }
        morph::Vector<T, M> spzl = this->sigmoid_prime_z_l();
        this->delta = w_times_delta.hadamard (spzl);

        // NB: We compute nabla_b and nabla_w on the OUTPUT neurons and the weights to
        // the output neurons..
        this->nabla_b = delta_l_nxt; // Size is N in Michael's code
        // nabla_w is a_in * delta_out, which means we compute a_in_last * delta? Or
        // we can compute nabla_w as a_in * delta_l_nxt here. I'm confused and need to
        // examine the example python code.
        for (size_t j = 0; j < N; ++j) {
            for (size_t i = 0; i < M; ++i) {
                this->nabla_w[i+(M*j)] = z[i] * delta_l_nxt[j]; // Size is M*N in Michael's code
            }
        }
    }
};

/*!
 * Holds data and methods for updating our neural network
 */
template <typename T>
struct FeedForwardNet
{
    FeedForwardNet() {
        // Set up initial conditions
        this->input.randomize();
        this->l1.randomize();
        this->output.zero();
        this->delta_out.zero();

        c1.in = &this->input;
        c1.out = &this->l1;

        c2.in = &this->l1;
        c2.out = &this->output;


        this->c1.randomize();
        this->c2.randomize();
    }

    //! Update the network's outputs from its inputs
    void update() {
        c1.compute();
        c2.compute();
    }

    //! Determine the error gradients by the backpropagation method. NB: Call
    //! computeCost() first
    void backprop() {
        // Notation follows http://neuralnetworksanddeeplearning.com/chap2.html
        // The output layer is special, as the error in the output layer is given by
        //
        // delta^L = grad_a(C) 0 sigma_prime(z^L)
        //
        // whereas for the intermediate layers
        //
        // delta^l = w^l+1 . delta^l+1 0 sigma_prime (z^l)
        //
        // (where 0 signifies hadamard product)
        // delta = dC_x/da() * sigmoid_prime(z_out)
        this->c2.backprop (this->delta_out); // c2.delta computed
        this->c1.backprop (this->c2.delta); // c1.delta computed
    }

    // Set up an input along with desired output
    void setInput (const morph::Vector<T, 784>& theInput, const morph::Vector<T, 10>& theOutput) {
        this->input = theInput;
        this->desiredOutput = theOutput;
    }

    //! Compute the cost for one input and one desired output
    T computeCost() {
        // Here is where we compute delta_out:
        this->delta_out = (desiredOutput-output).hadamard (c2.sigmoid_prime_z_lplus1()); // c2.z is the final activation
        // sum up the sos differences between output and desiredOutput
        T l = this->delta_out.length();
        this->cost = l * l;
        return this->cost;
    }

    // What's the cost function of the current output?
    T cost;

    // Because morph::Vectors have their size set at compile time, we have to specify each
    // layer manually. A template algorithm could be created to do this, but this is a
    // toy program, so it's manual.

    // 28x28 neurons in the input layer
    morph::Vector<T, 784> input;
    Connection<T, 784, MID_LAYER> c1;

    // MID_LAYER neurons in the hidden layer (as per the online example)
    morph::Vector<T, MID_LAYER> l1; // activations of l1 = sigmoid (c1.z)
    Connection<T, MID_LAYER, 10> c2;

    // 10 neurons in the output layer
    morph::Vector<T, 10> output; // activations of output layer - i.e. the output. = sigmoid(c2.z)
    morph::Vector<T, 10> delta_out;
    morph::Vector<T, 10> desiredOutput;
};
