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
    Vector<T, N> desout; // Desired output
    // Order of weights: in[0] to out[all], in[1] to out[all], in[2] to out[all] etc.
    Vector<T, MN> w;
    Vector<T, N> b;

    void randomize() {
        this->w.randomize();
        this->b.randomize();
    }

    //! out[i] = in[0,..,M] . w[i,..,i+M] + b[i]
    void compute() {
        // A Vector for a 'part of w'
        Vector<T, M> wpart;
        // Get weights, outputs and biases iterators
        auto witer = this->w.begin();
        auto oiter = this->out->begin();
        auto biter = this->b.begin();
        // Carry out an N sized for loop computing each output
        for (size_t j = 0; j < N; ++j) {
            // Copy part of weights to wpart:
            std::copy (witer, witer+M, wpart.begin());
            // Compute dot product with input and add bias:
            T presig = wpart.dot (*in) + *biter++;
            *oiter++ = T{1} / (T{1} + std::exp(-presig));
        }
    }

    Vector<T, M> backprop (const Vector<T, N>& dEdyj) {
        // Some notation here matches that in Plaut & Hinton 1987
        Vector<T, N> one_minus_out = -(*out) + T{1};
        // This is dEdyj * yj(1-yj) and is equivalent to delta_j in neuralnetworksanddeeplearning.com
        Vector<T, N> dEdxj = dEdyj.hadamard((*out).hadamard(one_minus_out));
        // In nn&dl.com, this is delta_out * a_in
        Vector<T, MN> dEdwji;
        for (size_t j = 0; j < N; ++j) {
            for (size_t i = 0; i < M; ++i) {
                dEdwji[M*j+i] = dEdxj[j] * (*in)[i];
            }
        }
        // Can now get dE/dyi for the next layer as sum_j dEdxj * wji or
        // dEdxj.dot(wpart) as it was in compute(). RETURN dEdyi (so it can be passed
        // to next layer)
        Vector<T, M> dEdyi;
        dEdyi.zero();
        for (size_t i = 0; i < M; ++i) {
            for (size_t j = 0; j < N; ++j) {
            // Note that we multiply dEdxj by this->w's elements i+0, i+M, i+2M,
            // i+3M,...,i+(N-1)M etc.
                dEdyi[i] += dEdxj[j] * this->w[i+(M*j)];
            }
        }
        return dEdyi;
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

    //! Update the network's outputs from its inputs
    void update() {
        c1.compute();
        c2.compute();
        c3.compute();
#if 0
        std::cout << output << std::endl;
#endif
    }

    //! Determine the error gradients by the backpropagation method. NB: Call
    //! computeCost() first
    void backprop() {
        // Step back through the layers, computing error gradients as we go
        dEdy_l2 = c3.backprop (dEdy_out);
        dEdy_l1 = c2.backprop (dEdy_l2);
        dEdy_in = c1.backprop (dEdy_l1);
#if 0
        std::cout << "dEdy_in: " << dEdy_in << std::endl;
        std::cout << "dEdy_l1: " << dEdy_l1 << std::endl;
        std::cout << "dEdy_l2: " << dEdy_l2 << std::endl;
        std::cout << "dEdy_out: " << dEdy_out << std::endl;
#endif
    }

    // Set up an input along with desired output
    void setInput (const Vector<T, 3>& theInput, const Vector<T, 2>& theOutput) {
        this->input = theInput;
        this->desiredOutput = theOutput;
    }

    //! Compute the cost for one input and one desired output
    T computeCost() {
        // sum up the sos differences between output and desiredOutput
        this->dEdy_out = desiredOutput - output;
        T l = this->dEdy_out.length();
        this->cost = l * l;
        return this->cost;
    }

    // What's the cost function of the current output?
    T cost;

    // Because Vectors have their size set at compile time, we have to specify each
    // layer manually. A template algorithm could be created to do this, but this is a
    // toy program, so it's manual. This is actually pretty awful and leads to a lot of
    // repeated code.
    Vector<T, 3> input;
    Vector<T, 3> dEdy_in;
    Connection<T, 3, 4> c1;

    Vector<T, 4> l1;
    Vector<T, 4> dEdy_l1;
    Connection<T, 4, 4> c2;

    Vector<T, 4> l2;
    Vector<T, 4> dEdy_l2;
    Connection<T, 4, 2> c3;

    Vector<T, 2> output;
    Vector<T, 2> dEdy_out;
    Vector<T, 2> desiredOutput;
};
