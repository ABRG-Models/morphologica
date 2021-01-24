# RecurrentNet

Implementation of the recurrent backpropagation algorithm (Pineda, 1987) for supervised learning with networks that have arbitrary topolgies (feed-forward, recurrent, random, etc.), with tools for easily loading input/target "maps", for discriminating between multiple maps via input to specfic "context nodes", and for representing/vizualising the activity on a hexagonal lattice.

## Quick start

Compile in the usual way (to create the recurrentnet program in the build directory):

```
cd recurrentnet
mkdir build
cd build
cmake ..
make
cd ..
```

Run using `python test.py`. 

This will first populate the test folder with map.h5  and network.h5, and then run an example network before also creating outputs.h5, weights.h5, and log.txt.

You can then run `./build/recurrentnet test 0 -2` to generate .png files showing the settled responses for each network node to four input patterns.  

Note that running test.py defines the structure of an input-output map and saves it into in test/map.h5, and this map corresponds to the classic XOR-problem. For the example, the response of node 2 (the network's output node) should be a 2x2 checkboard pattern, showing high/low responses in diagonal quadrants of the input space.

You can specify arbitrarily complicated maps for training/testing in this way, for example using values of X, Y, and F, to represent visual images. Running test.py also defines a network corresponding to that in Figure 1 of Pineda (1987 - full reference below), saved as a vector of pre-synaptic node identities and a vector of post-synaptic node identities into the file test/network.h5.   
  

## Overview

This project is essentially an implementation of the recurrent backpropagation algorithm for tranining neural networks, in a supervised manner to map a given set of inputs to a given set of outputs. The algorithm has the advantage over the commonly used backpropagation algorithm that it can be used to train networks with any topolgy (backprop is restricted to feed-forward networks only). Networks with recurrent connectivity can display attractor dynamics, hence it is important to allow the dynamics of recurrent networks to settle before connection weights are adjusted, and to detect when the dynamics have failed to settle. The resulting algorithm was introduced by Pineda in the following journal article:

Pineda, FJ. (1987) Generalization of back-propagation to recurrent neural networks. Physical Review Letters, 59, 2229.

The core algorithm, as reported by Pineda (1987) is implemented in RecurrentNetwork.h, which defines a RecurrentNetwork class, the methods of which can be called to iterate the activation and learning dynamics in response to pairs of inputs and target outputs. 

In addition RecurrentNetworkModel.h provides a useful interface to these methods, in order that inputs and target outputs can be imported from external files, and that the activation and learning can be easily visualised. An instance of class RecurrentNetworkModel provides access to:

i)      A vector of Map objects, each of which contains input/target pairs imported from external .h5 files; 
ii)     A Domain object, which contains a 2D hexagonal lattice that can be used to image the network activation;
iii)    The identity of network nodes that will be used as Input or Output nodes, or Context nodes (nodes that take specific input values when a given Map object is used for training/testing).




 


  
