# Dynamical model template

A basic template for a dynamical model that accepts user-defined model parameters (in params.json), outputs simulation data (to logdir/out.h5) that can be accessed offline (via analysis.py), and displays that data to a graph at runtime.  

Clone morphologica into this directory, then: 

`mkdir build`
`cd build`
`cmake ..`
`make`
`cd ..`
`./build/model params.json logs 0`
`python analysis`
