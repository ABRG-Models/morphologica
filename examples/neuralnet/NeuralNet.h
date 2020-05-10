/*!
 * \file
 *
 * This file contains a feed-forward neural network class, whose neuron sizes can be
 * configured at runtime and a class to hold the information about the connection
 * between layers of neurons in the network.
 *
 * \author Seb James
 * \date May 2020
 */

#include <morph/vVector.h>
#include <iostream>
#include <list>
#include <vector>
#include <sstream>
#include <ostream>

template <typename T>
struct Connection
{
    Connection (morph::vVector<T>* _in, morph::vVector<T>* _out) {
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

    //! Input layer has size M, output has size N. Weights size MbyN (or NbyM)
    morph::vVector<T>* in;
    size_t M = 0;
    //! Pointer to output layer. Size N
    morph::vVector<T>* out;
    size_t N = 0;
    //! The errors in the input layer of neurons. Size M
    morph::vVector<T> delta;
    //! Weights. Order of weights: in[0] to out[all], in[1] to out[all], in[2] to
    //! out[all] etc. Size M by N.
    morph::vVector<T> w;
    //! Biases. Size N.
    morph::vVector<T> b;
    //! The gradients of cost vs. weights. Size M by N.
    morph::vVector<T> nabla_w;
    //! The gradients of cost vs. biases. Size N.
    morph::vVector<T> nabla_b;
    //! Activation of the output neurons. Computed in feedforward, used in backprop
    //! z = sum(w.in) + b.
    morph::vVector<T> z; // N

    //! Output as a string
    std::string str() const {
        std::stringstream ss;
        ss << "Weights: w" << w << "w (" << w.size() << ")\n";
        ss << "nabla_w:nw" << nabla_w << "nw (" << nabla_w.size() << ")\n";
        ss << " Biases: b" << b << "b (" << b.size() << ")\n";
        ss << "nabla_b:nb" << nabla_b << "nb (" << nabla_b.size() << ")\n";
        return ss.str();
    }

    //! Randomize the weights and biases
    void randomize() {
        this->w.randomizeN (T{0.0}, T{1.0});
        this->b.randomizeN (T{0.0}, T{1.0});
    }

    //! Feed-forward compute. out[i] = in[0,..,M] . w[i,..,i+M] + b[i]
    void compute() {
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
            this->z[j] = wpart.dot (*in) + *biter++;
            *oiter++ = T{1} / (T{1} + std::exp(-z[j])); // out = sigmoid(z)
            // Move to the next part of the weight matrix for the next loop
            witer += this->M;
        }
    }

    //! The content of Connection::out is sigmoid(z^l+1)
    //! \return has size N
    morph::vVector<T> sigmoid_prime_z_lplus1() {
        return out->hadamard (-(*out)+T{1});
    }

    //! The content of Connection::in is sigmoid(z^l)
    //! \return has size M
    morph::vVector<T> sigmoid_prime_z_l() {
        return in->hadamard (-(*in)+T{1});
    }

    //! Compute this->delta using values computed in Connection::compute.
    void backprop (const morph::vVector<T>& delta_l_nxt/*, const morph::vVector<T>& z_prev*/) { // delta_l_nxt has size N.
        if (delta_l_nxt.size() != this->out->size()) {
            throw std::runtime_error ("backprop: Mismatched size");
        }
        // we have to do weights * delta_l_nxt to give a morph::vVector<T, N> result. This is
        // the equivalent of the matrix multiplication:
        morph::vVector<T> w_times_delta(this->in->size());
        w_times_delta.zero();
        for (size_t i = 0; i < this->M; ++i) { // Each input
            for (size_t j = 0; j < this->N; ++j) { // Each output
                w_times_delta[i] +=  this->w[i+(this->M*j)] * delta_l_nxt[j]; // For each weight fanning into neuron j in l_nxt, sum up
            }
        }
        morph::vVector<T> spzl = this->sigmoid_prime_z_l(); // spzl has size M; deriv of input
        this->delta = w_times_delta.hadamard (spzl);

        // NB: In a given connection, we compute nabla_b and nabla_w relating to the
        // OUTPUT neurons and the weights also relate to the output neurons.
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
std::ostream& operator<< (std::ostream& os, const Connection<T>& c)
{
    os << c.str();
    return os;
}

/*!
 * Holds data and methods for updating our neural network
 */
template <typename T>
struct FeedForwardNetS
{
    // Constructor with variable args to set layers and their contents? or just a vector?
    FeedForwardNetS(const std::vector<unsigned int>& layer_spec) {
        // Set up initial conditions
        for (auto nn : layer_spec) {
            morph::vVector<T> lyr(nn);
            lyr.zero();
            size_t lastLayerSize = 0;
            if (!this->neurons.empty()) {
                // Will add connection
                lastLayerSize = this->neurons.back().size();
            }
            this->neurons.push_back (lyr);
            if (lastLayerSize != 0) {
                auto l = this->neurons.end();
                --l;
                auto lm1 = l;
                --lm1;
                Connection<T> c(&*lm1, &*l);
                c.randomize();
                this->connections.push_back (c);
            }
        }
    }

    //! Output network as a string
    std::string str() const {
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
    void compute() {
        for (auto& c : this->connections) {
            c.compute();
        }
    }

    //! A function to just show the difference between out and desired out for debug
    void evaluate (const std::vector<morph::vVector<float>>& ins,
                   const std::vector<morph::vVector<float>>& outs) {
        auto op = outs.begin();
        for (auto ir : ins) {
            // Set input and output
            this->neurons.front() = ir;
            this->desiredOutput = *op++;
            // Compute network and cost
            this->compute();
            float c = this->computeCost();
            std::cout << "Input " << ir << " --> " << this->neurons.back() << " cf. " << this->desiredOutput << " (cost: " << c << ")\n";
        }
    }

    //! Evaluate against the Mnist test image set
    unsigned int evaluate (const std::multimap<unsigned char, morph::vVector<float>>& testData, int num=10000) {
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
            this->compute();
            evalcost += this->computeCost();
            // Success?
            //std::cout << "Comparing " << this->neurons.back() << " with target " << this->desiredOutput << " (key " << key << ")\n";
            if (this->argmax() == key) {
                ++numMatches;
            }
            ++count;
            if (count >= num) {
                break;
            }
        }
        return numMatches;
    }

    //! Find the element in output with the max value
    size_t argmax() {
        auto themax = std::max_element (this->neurons.back().begin(), this->neurons.back().end());
        size_t i = themax - this->neurons.back().begin();
        //std::cout << "max output of " << this->neurons.back() << " is element " << i << std::endl;
        return i;
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
        auto citer = this->connections.end();
        --citer; // Now points at output layer
        citer->backprop (this->delta_out); // Layer L delta computed
        for (;citer != this->connections.begin();) {
            auto citer_closertooutput = citer--;
            // Now citer is closer to input
            citer->backprop (citer_closertooutput->delta); // c1.delta computed
        }
    }

    //! Set up an input along with desired output
    void setInput (const morph::vVector<T>& theInput, const morph::vVector<T>& theOutput) {
        *(this->neurons.begin()) = theInput;
        this->desiredOutput = theOutput;
    }

    //! Compute the cost for one input and one desired output
    T computeCost() {
        // Here is where we compute delta_out:
        this->delta_out = (this->neurons.back()-desiredOutput).hadamard (this->connections.back().sigmoid_prime_z_lplus1()); // c2.z is the final activation
#if 0
        std::cout << "Desired out: "<< desiredOutput << std::endl;
        std::cout << "neurons.back(): "<< this->neurons.back() << std::endl;
        std::cout << "desiredOut - neurons.back(): "<< (desiredOutput-this->neurons.back()) << std::endl;
        //std::cout << "connections.back(): "<< this->connections.back() << std::endl;
        std::cout << "connections.back().sigmoid_prime_z_lplus1(): "<< this->connections.back().sigmoid_prime_z_lplus1() << std::endl;
        std::cout << "delta_out: " << this->delta_out << ", with length " << this->delta_out.length() << std::endl;
#endif
        // sum up the sos differences between output and desiredOutput to get the cost
        T l = (desiredOutput-this->neurons.back()).length();
        this->cost = T{0.5} * l * l;
        return this->cost;
    }

    // What's the cost function of the current output?
    T cost = T{0};

    // Variable number of neuron layers, each of variable size.
    std::list<morph::vVector<T>> neurons;
    // Should be neurons.size()-1 connection layers
    std::list<Connection<T>> connections;

    morph::vVector<T> delta_out; // error (dC/dz) of the output layer
    morph::vVector<T> desiredOutput;
};

template <typename T>
std::ostream& operator<< (std::ostream& os, const FeedForwardNetS<T>& ff)
{
    os << ff.str();
    return os;
}
