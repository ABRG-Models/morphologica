#include "Mnist.h"
#include <morph/Random.h>

#define MID_LAYER 30
#include "NeuralNet.h"

int main()
{
    // Read the MNIST data
    Mnist m;

    // Instantiate the network (What would be really cool would be a FeedForwardNet as a
    // variadic template, so that FeedForwardNet<float, 785, 15, 10> ff1 would create
    // the network the right way)
    FeedForwardNet<float> ff1;

    // Create a random number generator
    morph::RandUniform<unsigned char> rng((unsigned char)0, (unsigned char)9);

    // main loop, while m.training_f has values in:
    unsigned int epochs = 30;
    unsigned int mini_batch_size = 10;
    float eta = 3.0;

    // Do this several times, accumulate the errors, then update the weights/biases

    // One epoch is one time through the whole dataset
    float cost = 0.0f;
    for (unsigned int e = 0; e < epochs; ++e) {
        std::cout << "Epoch " << e << ", latest cost: " << cost << std::endl;
        // Copy out the training data:
        std::multimap<unsigned char,
                      morph::Vector<float, mnlen>> training_f = m.training_f;

        for (unsigned int j = 0; j < training_f.size()/mini_batch_size; ++j) {

            // Accumulate the dC/dw and dC/db values
            morph::Vector<float, MID_LAYER> c1_nabla_b;
            c1_nabla_b.zero();
            morph::Vector<float, 784*MID_LAYER> c1_nabla_w;
            c1_nabla_w.zero();
            morph::Vector<float, 10> c2_nabla_b;
            c2_nabla_b.zero();
            morph::Vector<float, MID_LAYER*10> c2_nabla_w;
            c2_nabla_w.zero();
            cost = 0.0f;

            for (unsigned int i = 0; i < mini_batch_size; ++i) {
                auto t_iter = training_f.find (rng.get());
                unsigned int key = static_cast<unsigned int>(t_iter->first);
                //std::cout << "The key is " << key << std::endl;
                morph::Vector<float, 784> thein = t_iter->second;
                thein.zero();
                morph::Vector<float, 10> theout = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
                theout[key] = 1;
                training_f.erase (t_iter);
                ff1.setInput (thein, theout);
                ff1.update();
                cost += ff1.computeCost();
                ff1.backprop();

                // Now collect up the cost, the nabla_w and nabla_bs for the learning step
                c1_nabla_b += ff1.c1.nabla_b;
                c1_nabla_w += ff1.c1.nabla_w;
                c2_nabla_b += ff1.c2.nabla_b;
                c2_nabla_w += ff1.c2.nabla_w;
#if 0
                std::cout << "c1 nabla b: " << ff1.c1.nabla_b << std::endl;
                std::cout << "c1 nabla w: " << ff1.c1.nabla_w << std::endl;
                std::cout << "c2 nabla b: " << ff1.c2.nabla_b << std::endl;
                std::cout << "c2 nabla w: " << ff1.c2.nabla_w << std::endl;
#endif
            }
            c1_nabla_b /= mini_batch_size;
            c1_nabla_w /= mini_batch_size;
            c2_nabla_b /= mini_batch_size;
            c2_nabla_w /= mini_batch_size;
            cost /= (2*mini_batch_size);

            //std::cout << "Cost = " << cost << std::endl;

            // Gradient update. v -> v' = v - eta * gradC
            ff1.c1.w -= (c1_nabla_w * eta);
            ff1.c1.b -= (c1_nabla_b * eta);
            ff1.c2.w -= (c2_nabla_w * eta);
            ff1.c2.b -= (c2_nabla_b * eta);

        }
    }
    return 0;
}
