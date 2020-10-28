# Elman results

This shows the results I obtained with elman1.cpp.

## elman_mean_half_e_squared.png

This shows the Elman 1990, Fig. 3 result with, as far as I can tell,
the parameters set as described by the paper. I think that the error
computed and presented in Fig. 3 is the 'mean of 0.5 * error * error',
where error is the difference between the predicted output of the
network (prediction stream or ps in the code) and the binary output of
the network, which is 0 unless the output neuron value is greater than
0.5, in which case it is 1.

The blue line is the result; the green line results from computing the
error as above on two random bit streams.

## elman_applied_to_prediction_or_truexor.png

This shows the same result as elman_mean_half_e_squared.png, but also
the result of comparing the network output with a random bit stream
and the result of comparing the network output with the 'running XOR
of the preceding two bits of the input'. This (right hand graph)
demonstrates that the network is always trying to XOR its current and
its previous inputs.

## elman_root_mean_e_squared.png

I think that the paper text suggests that the error presented in
Fig. 3 is the 'root of the mean of the square of the error'. This
image shows the graphs of elman_mean_half_e_squared.png, with the
addition of an RMS result. Note that the y axis extends from 0 to 1,
rather than from 0 to 0.5. On this basis, I think that 'Error' in
Fig. 3 in Elman's paper is the 'mean of 0.5 * error * error'.
