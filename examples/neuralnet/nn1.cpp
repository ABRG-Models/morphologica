#include "NeuralNet.h"
//#include "Mnist.h"

int main()
{
//    Mnist m;
    FeedForwardNet<float> ff1;

    Vector<float, 784> thein;
    thein.randomize();
    Vector<float, 10> theout = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    ff1.setInput (thein, theout);

    for (unsigned int i = 0; i < 10; ++i) {
        ff1.update();
        float c = ff1.computeCost();
        ff1.backprop();
    }

    return 0;
}
