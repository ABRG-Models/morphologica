#include <morph/nn/ElmanNet.h>
#include <iostream>
#include <morph/Random.h>
#include <morph/vvec.h>

// Prepare XOR sequence (xs) and prediction sequence (ps). The XOR sequence is computed
// as in Elman, 1990. Create two bits at random, then xor the bits to make the third
// bit. Create another two bits at random and then insert the XOR of these as the sixth
// bit. Repeat xs_trips times. The sequence ps[i] is simply the value of xs[i+1].
void generateInput (size_t xs_trips, morph::vvec<float>& xs, morph::vvec<float>& ps, morph::vvec<float>& xl)
{
    xs.resize (xs_trips*3, 0.0f);
    ps.resize (xs_trips*3, 0.0f);
    xl.resize (xs_trips*3, 0.0f);
    morph::RandUniform<unsigned long long int> rng;
    size_t j = 0;
    for (size_t i = 0; i < xs_trips;) {

        // Can get 2*64 bits of random at a time:
        unsigned long long int left = rng.get();
        unsigned long long int right = rng.get();

        // Change this line to experiment with other logical ops - replace ^ with | or &
        unsigned long long int xor_ = left ^ right;   // XOR

        // For each of the 64 pairs of randomness in left & right, add 3 elements to xs, ps and xl
        for (size_t ii = 0; i<xs_trips && ii<64; ii++,i++) {
            unsigned long long int l = left >> ii & 0x1;
            unsigned long long int r = right >> ii & 0x1;
            unsigned long long int x = xor_ >> ii & 0x1;

            // First bit (if possible)
            if (j) { ps[j-1] = l > 0 ? 1.0f : 0.0f; }
            if (j>1) {
                xl[j-1] = ( (xs[j-2] > 0.0f || xs[j-1] > 0.0f)
                            && !(xs[j-2] > 0.0f && xs[j-1] > 0.0f) ) ? 1.0f : 0.0f;
            }
            // the first bit of xs is 'left'
            xs[j++] = l > 0 ? 1.0f : 0.0f;

            // Second bit
            ps[j-1] = r > 0 ? 1.0f : 0.0f;
            xl[j-1] = ( (xs[j-2] > 0.0f || xs[j-1] > 0.0f)
                        && !(xs[j-2] > 0.0f && xs[j-1] > 0.0f) ) ? 1.0f : 0.0f;
            // second bit of xs is 'right'
            xs[j++] = r > 0 ? 1.0f : 0.0f;

            // Third bit
            ps[j-1] = x > 0 ? 1.0f : 0.0f;
            xl[j-1] = ( (xs[j-2] > 0.0f || xs[j-1] > 0.0f)
                        && !(xs[j-2] > 0.0f && xs[j-1] > 0.0f) ) ? 1.0f : 0.0f;
            // the third bit of xs is 'left' XOR 'right'
            xs[j++] = x > 0 ? 1.0f : 0.0f;
        }
    }
}

int main()
{
    //
    // Simulation parameters
    //

    // The network specification. Hidden layers have an associated context layer, so
    // layer_spec = {1,2,1} gives a network with 1 neuron in the input layer, 2 neurons
    // in the hidden layer, 2 in the context layer and 1 in the output layer.
    std::vector<unsigned int> layer_spec = {1,2,1};
    // A learning rate. I can't find the learning rate used in Elman 1990 for Fig. 3
    float eta = 0.1f;
    // epochs defines the number of times to run through the data stream during training
    size_t epochs = 600;

    //
    // Create an Elman style feed-forward network with context layers
    //
    morph::nn::ElmanNet<float> el1(layer_spec);

    //
    // Prepare the input
    //

    // Number of 'triplets' in the XOR sequence (triplet comprises 'in', 'in' and 'correct out' bits.
    size_t xs_trips = 1000;
    // The XOR sequence container
    morph::vvec<float> xs;
    // The prediction sequence container - to reproduce Elman 1990, Fig. 3
    morph::vvec<float> ps;
    // The result of XORing the last two bits each time - to verify that trained Elman net implements XOR
    morph::vvec<float> xl;
    // populate xs, ps and xl:
    generateInput (xs_trips, xs, ps, xl);

    // Note that the weight update step of the training is *not* built into
    // morph::ElmanNet, and so we will accumulate the dC/dw and dC/db gradient values
    // computed by the backprop into this container 'gradients'. For each std::pair in
    // gradients, 'first' is nabla_w and 'second' is nabla_b (there are as many pairs as
    // there are connections in el1).
    std::vector<std::pair<std::vector<morph::vvec<float>>, morph::vvec<float>>> gradients;
    for (auto& c : el1.connections) { gradients.push_back (std::make_pair(c.nabla_ws, c.nabla_b)); }

    //
    // Train the network
    //

    // Containers to pass as input and desired output. Values from xs and ps are copied into these, shorter vectors.
    morph::vvec<float> input = {1};
    morph::vvec<float> des_output = {1};
    // For each epoch run through the length of xs/ps
    for (size_t ep = 0; ep<epochs; ++ep) {
        for (size_t i = 0; i < xs.size(); ++i) {

            // Zero the gradient containers
            size_t gi = 0;
            for (gi = 0; gi < el1.connections.size(); ++gi) {
                for (size_t j = 0; j < gradients[gi].first.size(); ++j) {
                    gradients[gi].first[j].zero();
                }
                gradients[gi].second.zero();
            }

            // Set the new input and desired output
            input[0] = xs[i];
            des_output[0] = ps[i];
            el1.setInput (input, des_output);

            // Compute the network forwards
            el1.feedforward();
            el1.computeCost();

            // Back-propagate the error to find the gradients of the weights and biases
            el1.backprop();

            // Weight update based on backprop from one run (with no batching).
            //
            // Copy the gradients from the network into the container (this step would
            // look more relevant/sensible if we were training in batches).
            gi = 0;
            for (auto& c : el1.connections) {
                for (size_t j = 0; j < gradients[gi].first.size(); ++j) {
                    gradients[gi].first[j] += c.nabla_ws[j];
                }
                gradients[gi].second += c.nabla_b;
                ++gi;
            }
            // perform the weight update based on the gradients. v -> v' = v - eta * gradC
            gi = 0;
            for (auto& c : el1.connections) {
                // for each in weights:
                for (size_t j = 0; j < gradients[gi].first.size(); ++j) {
                    c.ws[j] -= (gradients[gi].first[j] * eta);
                }
                c.b -= (gradients[gi].second * eta);
                ++gi;
            }
        }
    }

    //
    // Evaluate the network, averaging over 1200 elements, as in Elman, 1990.
    //

    size_t eval_elements = 1200;
    size_t graph_cycles = 12;

    // Create 2 more truly random binary strings for testing/debug
    morph::RandUniform<unsigned short> brng(0,1);

    // Random string 1
    morph::vvec<float> rs1;
    rs1.resize(eval_elements);
    for (auto& r : rs1) {
        unsigned short rn = brng.get();
        r = rn > 0 ? 1.0f : 0.0f;
    }
    // Random string 2
    morph::vvec<float> rs2;
    rs2.resize(eval_elements);
    for (auto& r : rs2) {
        unsigned short rn = brng.get();
        r = rn > 0 ? 1.0f : 0.0f;
    }

    // Containers for the results
    morph::vvec<float> costs(graph_cycles, 0.0f);
    morph::vvec<float> randcosts(graph_cycles, 0.0f);

    for (size_t i = 0; i < eval_elements; ++i) {
        // Set the new input and desired output
        input[0] = xs[i];
        // The desired output is the prediction stream - to reproduce Elman 1990, Fig. 3.
        des_output[0] = ps[i];
#if 0
        // if you compare with the random stream, costs should be similar to randcosts
        des_output[0] = rs[i];
        // The actual XOR of each preceding two elements. With this as desired output,
        // you should see costs at the minimum value achieved by the prediction stream.
        des_output[0] = xl[i];
#endif
        el1.setInput (input, des_output);
        // Compute the network fowards:
        el1.feedforward();
        // sums the error from the network
        costs[i%graph_cycles] += el1.computeCost();

        // Compute an alternative cost, based on two streams of random numbers for comparison
        float e = (float)rs1[i] - (float)rs2[i];
        randcosts[i%graph_cycles] += morph::nn::ElmanNet<float>::costKernel (e);
    }

    // Divide to obtain the mean of the squared error in each element of costs/randcosts
    costs /= (eval_elements/graph_cycles);
    randcosts /= (eval_elements/graph_cycles);

#if 0
    // I don't think Elman presents the root of the mean of the squared error for Fig. 3, so no sqrt here.
    costs.sqrt_inplace();
    randcosts.sqrt_inplace();
#endif

    // Output results on stdout
    std::cout << "\ncosts=" << costs.str_mat() << std::endl;
    std::cout << "randcosts= " << randcosts.str_mat() << std::endl;

    std::cout << "\ncosts min: " << costs.min() << ", max: " << costs.max() << std::endl;
    std::cout << "randcosts min: " << randcosts.min() << ", max: " << randcosts.max() << std::endl;

    return 0;
}
