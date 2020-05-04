/*!
 * \file
 *
 * A toy neural net, for a NN example
 */

#include <vector>
#include <list>
#include <morph/Vector.h>
#include <iostream>

// using directives for later removal
using std::vector;
using std::list;
using morph::Vector;

//! A connection between a source layer of size M and a destination layer of size N
template <typename T, size_t M, size_t N, size_t MN=M*N>
struct Connection
{
    Vector<T, M>* in;  // Pointer to input layer
    Vector<T, N>* out; // Pointer to output layer
    // Order of weights: in[0] to out[all], in[1] to out[all], in[2] to out[all] etc.
    Vector<T, MN> w;
    Vector<T, N> b;

    void randomize() {
        this->w.randomize();
        this->b.randomize();
    }

    //! out[i] = in[0,..,M] . w[i,..,i+M] + b[i]
    void compute() {
        // Copy a part of w into a separate Vector
        Vector<T, M> wpart;
        // Get weights, outputs and biases iterators
        auto witer = this->w.begin();
        auto oiter = this->out->begin();
        auto biter = this->b.begin();
        // Carry out an N sized for loop computing each output
        for (size_t i = 0; i < N; ++i) {
            // Copy part of weights to wpart:
            std::copy (witer, witer+M, wpart.begin());
            // Compute dot product with input and add bias:
            T presig = wpart.dot (*in) + *biter++;
            *oiter++ = T{1} / (T{1} + std::exp(-presig));
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
        c1.in = &this->input;
        c1.out = &this->l1;

        c2.in = &this->l1;
        c2.out = &this->l2;

        c3.in = &this->l2;
        c3.out = &this->output;

        this->input.randomize();
        this->l1.randomize();
        this->l2.randomize();
        this->output.zero();

        this->c1.randomize();
        this->c2.randomize();
        this->c3.randomize();
    }

    void update() {
        // Update the network
        c1.compute();
        c2.compute();
        c3.compute();
        std::cout << output << std::endl;
    }

    // Set up an input
    void setInput() {
    }

    // Compute the cost for one input and one desired output
    void computeCost() {
        // sum up the sos differences between output and desiredOutput
        Vector<T, 2> outdiff = desiredOutput - output;
        T l = outdiff.length();
        this->cost = l * l;
    }

    // What are we shooting for?
    Vector<T, 2> desiredOutput;

    // What's the cost function of the current output?
    T cost;

    // Because Vectors have their size set at compile time, we have to specify each
    // layer manually. A template algorithm could be created to do this, but this is a
    // toy program, so it's manual. This is actually pretty awful and leads to a lot of
    // repeated code.
    Vector<T, 3> input;
    Connection<T, 3, 4> c1;
    Vector<T, 4> l1;
    Connection<T, 4, 4> c2;
    Vector<T, 4> l2;
    Connection<T, 4, 2> c3;
    Vector<T, 2> output;
};
