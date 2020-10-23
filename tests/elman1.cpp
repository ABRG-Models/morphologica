#include <morph/nn/ElmanNet.h>
#include <iostream>

int main()
{
    // Create an Elman style feed-forward network with context layers
    std::vector<unsigned int> layer_spec = {1,2,1};
    morph::nn::ElmanNet<float> el1(layer_spec);
    std::cout << el1.str();
    el1.feedforward();
    std::cout << "After feedforward():\n";
    std::cout << el1.str();
    return 0;
}
