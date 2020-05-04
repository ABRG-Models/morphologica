#include "NeuralNet.h"
#include "Mnist.h"

int main()
{
    Mnist m;
    FeedForwardNet<float> ff1;
    // Implement a backpropagation learning algorithm which will include:
    ff1.update();
    return 0;
}
