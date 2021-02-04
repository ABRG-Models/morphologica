import numpy as np
import pylab as pl
import h5py

h = h5py.File('logs/out.h5','r')
x = h['X'][:]
x = h['Y'][:]
h.close()

F = pl.figure()
f = F.add_subplot(111)
f.plot(x)

pl.show()
