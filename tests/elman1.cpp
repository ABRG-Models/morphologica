#include <morph/nn/ElmanNet.h>
#include <iostream>
#include <morph/Random.h>
int main()
{
    //
    // Create an Elman style feed-forward network with context layers
    //
    std::vector<unsigned int> layer_spec = {1,2,1};
    morph::nn::ElmanNet<float> el1(layer_spec);

    //
    // Prepare the input
    //
    morph::vVector<float> xs; // XOR sequence. Starting with 1, 0. Computed as in Elman,
                              // 1990. Take two bits at random, then xor the bits to
                              // make the third bit. Add these three bits to the
                              // seqeunce and start again.

    morph::vVector<float> ps; // Prediction sequence

    size_t xs_trips = 10; // XOR sequence triplets consisting of in, in and correct out.
    xs.resize (xs_trips*3, 0.0f);
    ps.resize (xs_trips*3, 0.0f);
    morph::RandUniform<unsigned long long int> rng;
    size_t j = 0;
    for (size_t i = 0; i < xs_trips;) {
        // Can get 64 bits of random at a time:
        unsigned long long int left = rng.get();
        unsigned long long int right = rng.get();
        unsigned long long int xor_ = left ^ right;
        // Get the relevant bits from the random number generator and insert into xs
        for (size_t ii = 0; i<xs_trips && ii<64; ii++,i++) {
            unsigned long long int l = left >> ii & 0x1;
            unsigned long long int r = right >> ii & 0x1;
            unsigned long long int x = xor_ >> ii & 0x1;
            if (j) { ps[j-1] = l > 0 ? 1.0f : 0.0f; }
            xs[j++] = l > 0 ? 1.0f : 0.0f;

            ps[j-1] = r > 0 ? 1.0f : 0.0f;
            xs[j++] = r > 0 ? 1.0f : 0.0f;

            ps[j-1] = x > 0 ? 1.0f : 0.0f;
            xs[j++] = x > 0 ? 1.0f : 0.0f;
        }
    }
    //std::cout << "XOR stream: " << xs << std::endl;

    // Containers to pass as input and desired output
    morph::vVector<float> input = {1};
    morph::vVector<float> des_output = {1};

    // Accumulate the dC/dw and dC/db values. for each pair, the first is
    // nabla_w the second is nabla_b. There are as many pairs as there are connections
    // in ff1. Here, we declare and initialize mean_gradients
    std::vector<std::pair<std::vector<morph::vVector<float>>, morph::vVector<float>>> mean_gradients;
    for (auto& c : el1.connections) {
        mean_gradients.push_back (std::make_pair(c.nabla_ws, c.nabla_b));
    }

    //
    // Train the network
    //

    // A learning rate
    float eta = 0.01f;
    // How many times to run through the data stream?
    size_t epochs = 600;

    for (size_t ep = 0; ep<epochs; ++ep) {
        //float cost = 0.0f;
        //std::cout << "Epoch START:\n" << el1 << std::endl;

        for (size_t i = 0; i < xs.size(); ++i) {

            // Zero the mean gradents
            size_t gi = 0;
            for (gi = 0; gi < el1.connections.size(); ++gi) {
                for (size_t j = 0; j < mean_gradients[gi].first.size(); ++j) {
                    mean_gradients[gi].first[j].zero();
                }
                mean_gradients[gi].second.zero();
            }

            // Set the new input and desired output
            input[0] = xs[i];
            des_output[0] = ps[i];
            el1.setInput (input, des_output);

            // Compute the network fowards
            el1.feedforward();
            /*cost =*/ el1.computeCost(); // Could make this a part of feedforward

            // Back propagate the error to find the gradients of the weights and biases
            el1.backprop();

            // Copy gradients into mean_gradients (which isn't a mean)
            gi = 0;
            for (auto& c : el1.connections) {
                for (size_t j = 0; j < mean_gradients[gi].first.size(); ++j) {
                    mean_gradients[gi].first[j] += c.nabla_ws[j];
                }
                mean_gradients[gi].second += c.nabla_b;
                ++gi;
            }

            // Examine a single epoch:
            //std::cout << ep << "," << i << "," << cost << "," << input[0] << "," << des_output[0] << std::endl;

            // perform the gradient update. v -> v' = v - eta * gradC
            gi = 0;
            for (auto& c : el1.connections) {
                // for each in weights:
                for (size_t j = 0; j < mean_gradients[gi].first.size(); ++j) {
                    //std::cout << "Updating connection weights:" << c.ws[j] << std::endl;
                    c.ws[j] -= (mean_gradients[gi].first[j] * eta);
                }

                c.b -= (mean_gradients[gi].second * eta);
                //std::cout << "c = " << c << std::endl;
                ++gi;
            }
        }
        //std::cout << "Epoch END:\n" << el1 << std::endl;
        // Output at end of each epoch
        //std::cout << ep << "," << xs.size() << "," << cost  << "," << input[0] << "," << des_output[0] << std::endl;
    }

    //
    // Evaluate the network
    //
    morph::vVector<float> costs(12);
    for (size_t i = 0; i < xs.size(); ++i) {
        // Set the new input and desired output
        input[0] = xs[i];
        des_output[0] = ps[i];
        el1.setInput (input, des_output);
        // Compute the network fowards
        el1.feedforward();
        float cost = el1.computeCost();
        costs[i%12] += cost;
    }

    costs /= (xs.size()/12);
    std::cout << "costs: " << costs << std::endl;

    return 0;
}
