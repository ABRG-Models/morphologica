#include <morph/nn/ElmanNet.h>
#include <morph/vvec.h>
#include <vector>
#include <iostream>

int main()
{
    std::vector<unsigned int> layer_spec = {1,2,1};
    morph::nn::ElmanNet<float> el1(layer_spec);

    // Manually set weights. These from a trained network.
    /*
Layer 0 neurons:  (0)
Connection:
 Input 0: Weights: w(-3.15769,-6.00931)w (2)
 Input 1: Weights: w(1.36145,3.11116,2.91738,4.21558)w (4)
 Input 0: nabla_w:nw(0,-0)nw (2)
 Input 1: nabla_w:nw(0.000852796,0.000844218,-0.00116073,-0.00114906)nw (4)
 Output Biases: b(1.27834,-2.05479)b (2)
 Output nabla_b:nb(0.000858184,-0.00116806)nb (2)
 Input 0: delta  :  (0)
 Input 1: delta  :  (-1.39678e-05,-3.60823e-05)
Layer 1 neurons:  (0.996639,0.993246)
Layer 1 context:  (0.993722,0.983726)
Connection:
 Input 0: Weights: w(3.14941,-2.15443)w (2)
 Input 0: nabla_w:nw(0.0808554,0.0805802)nw (2)
 Output Biases: b(-1.62233)b (1)
 Output nabla_b:nb(0.0811281)nb (1)
 Input 0: delta  :  (0.000858184,-0.00116806)
Layer 2 neurons:  (0.354524)
Target output: (0)
Delta out: (0.0811281)
Cost:      0
    */
    std::list<morph::nn::FeedForwardConn<float>>::iterator ci = el1.connections.begin();
    ci->ws[0] = {-3.15769,-6.00931};
    ci->ws[1] = {1.36145,3.11116,2.91738,4.21558};
    ci->b = {1.27834,-2.05479};
    ci++;
    ci->ws[0] = {3.14941,-2.15443};
    ci->b = {-1.62233};

    float in1 = 1.0f;
    float in2 = 0.0f;
    float in1xorin2 = 1.0f;

    morph::vvec<float> input = {1};
    morph::vvec<float> des_output = {1};

    // Set the new input and desired output
    input[0] = in1;
    des_output[0] = 0;
    el1.setInput (input, des_output);
    el1.feedforward();
    el1.computeCost();

    // Set input again, and expect the network to give us 1 as output (1 xor 0 = 1)
    input[0] = in2;
    des_output[0] = in1xorin2;
    el1.setInput (input, des_output);
    el1.feedforward();
    el1.computeCost();

    // Back-propagate the error to find the gradients of the weights and biases
    el1.backprop();

    std::cout  << "Network:\n" << el1;

    std::cout << std::endl << in1 << " then " << in2 << " presented to network gives output: " << el1.neurons.back() << std::endl;

    int rtn = -1;
    if (el1.neurons.back()[0] - 0.643637f < 0.000001f) {
        rtn = 0;
    }

    // Should also test/examine gradients
#if 0
    size_t gi = 0;
    for (auto& c : el1.connections) {
        for (size_t j = 0; j < gradients[gi].first.size(); ++j) {
            gradients[gi].first[j] += c.nabla_ws[j];
        }
        gradients[gi].second += c.nabla_b;
        ++gi;
    }
#endif
    return rtn;
}
