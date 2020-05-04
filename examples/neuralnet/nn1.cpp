#include "NeuralNet.h"
#include "Mnist.h"

int main()
{
//    Mnist m;
    FeedForwardNet<float> ff1;

    Vector<float, 3> thein = {0.4,0.5,0.6};
    Vector<float, 2> theout = {1, 0};

    ff1.setInput (thein, theout);

    for (unsigned int i = 0; i < 10000000; ++i) {
        ff1.update();
        float c = ff1.computeCost();
        ff1.backprop();
    }

#if 0 // This'll be the algorithm:

    // The forward pass; computing cost for a number of training inputs
    float total_cost = 0;
    for (auto in : m.subset()) {

        // Forward pass
        ff1.setInput (in);
        ff1.update();
        total_cost += ff1.computeCost();
        // Backward pass, computing gradients
        ff1.backprop(); // somehow save gradients for this input vector

    }
#endif

    return 0;
}
