#include "Mnist.h"
#include <morph/Random.h>
#include "NeuralNet.h"

int main()
{
    // Read the MNIST data
    Mnist m;
    // Instantiate the network (What would be really cool would be a FeedForwardNet as
    // a variadic template, so that FeedForwardNet<float, 785, 15, 10> ff1 would
    // create the network the right way)
    FeedForwardNet<float> ff1;
    // Create a random number generator
    morph::RandUniform<unsigned char> rng((unsigned char)0, (unsigned char)9);

    // main loop, while m.training_f has values in:
    unsigned int epochs = 10;
    unsigned int mini_batch_size = 10;
    float eta = 1.0;

    // Do this several times, accumulate the errors, then update the weights/biases
    auto t_iter = m.training_f.find (rng.get());
    unsigned int key = static_cast<unsigned int>(t_iter->first);
    std::cout << "The key is " << key << std::endl;
    morph::Vector<float, 784> thein = t_iter->second;
    thein.zero();
    morph::Vector<float, 10> theout = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    theout[key] = 1;
    m.training_f.erase (t_iter);
    ff1.setInput (thein, theout);
    ff1.update();
    float c = ff1.computeCost();
    ff1.backprop();

    ff1.updateSomehow();

    return 0;
}
