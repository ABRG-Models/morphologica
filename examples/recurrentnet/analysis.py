# -*- coding: utf-8 -*-

import h5py
import numpy as np
import pylab as pl

# Parameters
dir = 'test' # This folder should exist. It should contain a config.json file
N = 5
outputID = 2

h5f = h5py.File(dir+'/outputs.h5','r')
r = h5f['responses'][:]
e = h5f['error'][:]
h5f.close()

# 'response' r is (num. units) x (num. map points)
M = int(len(r)/N)
m = int(np.sqrt(M))
r = r.reshape(N,M)

F = pl.figure(figsize=(10,5))
f = F.add_subplot(121)
f.plot(np.arange(len(e))*1000,e)
f.set_xlabel('learning iterations')
f.set_ylabel('error')

f = F.add_subplot(122)
im = f.imshow(np.reshape(r[outputID,:],(m,m)))
f.set_title('response of node '+str(outputID))
f.set_xlabel('input 1')
f.set_ylabel('input 2')
F.colorbar(im,ax=f)

pl.show()




