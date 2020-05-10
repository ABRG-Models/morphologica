# Feedforward neural net

The code in FeedForward.h and ff_mnist.cpp implements a three layer
feed forward neural network (785-30-10 neurons) just like Michael
Niemann's network at http://neuralnetworksanddeeplearning.com. The
network learns to characterize the well known MNIST database of
hand-written numerals, achieving a 94% success rate.

Build in the usual cmake manner:

```bash
mkdir build
cd build
cmake ..
make -j4
cd ..
./build/ff_mnist # You have to run from one back, so the program can load data from ./mnist/
```
