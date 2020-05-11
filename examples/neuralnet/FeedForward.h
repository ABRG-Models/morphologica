/*!
 * \file
 *
 * This file contains two classes: a feed-forward neural network class, whose neuron
 * sizes can be configured at runtime and a class to hold the information about the
 * connections between layers of neurons in the network.
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

/*!
 * A connection between neuron layers in a simple, stacked neural network.
 */
template <typename T>
struct FeedForwardConn
{
    FeedForwardConn (morph::vVector<T>* _in, morph::vVector<T>* _out) {
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
    void feedforward() {
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

    //! The content of *FeedForwardConn::out is sigmoid(z^l+1)
    //! \return has size N
    morph::vVector<T> sigmoid_prime_z_lplus1() {
        return (*out) * (-(*out)+T{1});
    }

    //! The content of *FeedForwardConn::in is sigmoid(z^l)
    //! \return has size M
    morph::vVector<T> sigmoid_prime_z_l() {
        return (*in) * (-(*in)+T{1});
    }

    /*!
     * Compute this->delta using the values computed in FeedForwardConn::feedforward
     * (which must have been executed beforehand).
     */
    void backprop (const morph::vVector<T>& delta_l_nxt) { // delta_l_nxt has size N.
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

/*!
 * A feedforward network class which holds a runtime-selectable set of neuron layers
 * and teh connections between the layers. Note that in this class, the connections
 * are always between adjacent layers; from layer l to layer l+1.
 */
template <typename T>
struct FeedForwardNet
{
    //! Constructor takes a vector specifying the number of neurons in each layer (\a
    //! layer_spec)
    FeedForwardNet (const std::vector<unsigned int>& layer_spec) {
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
                FeedForwardConn<T> c(&*lm1, &*l);
                c.randomize();
                this->connections.push_back (c);
            }
        }
    }

    //! Output the network as a string
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
    void feedforward() {
        for (auto& c : this->connections) {
            c.feedforward();
        }
    }

    //! A function which shows the difference between the network output and
    //! desiredOutput for debugging
    void evaluate (const std::vector<morph::vVector<float>>& ins,
                   const std::vector<morph::vVector<float>>& outs) {
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
    void setInput (const morph::vVector<T>& theInput, const morph::vVector<T>& theOutput) {
        *(this->neurons.begin()) = theInput;
        this->desiredOutput = theOutput;
    }

    //! Compute the cost for one input and one desired output
    T computeCost() {
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
    std::list<FeedForwardConn<T>> connections;
    //! The error (dC/dz) of the output layer
    morph::vVector<T> delta_out;
    //! The desired output of the network
    morph::vVector<T> desiredOutput;
};

template <typename T>
std::ostream& operator<< (std::ostream& os, const FeedForwardNet<T>& ff)
{
    os << ff.str();
    return os;
}
