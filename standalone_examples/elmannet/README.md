# Elman recurrent network

The program elman1 implements the result presented in Fig. 3 of

Elman, J.L. "Finding Structure in Time", Cognitive Science 14, 179-211
(1990)

It uses the definition of the Elman network in morph::nn::ElmanNet,
and implements training and evaluation of the result, as I interpreted
it from the paper.

To build and run:

```sh
mkdir build && cd build
cmake ..
make
./elman1
```

The output is an array of costs, which can be graphed in your graphing
program of choice (once I've fixed text in morph::Visual, this program
might draw the graph itself).

Seb James, October 2020.
