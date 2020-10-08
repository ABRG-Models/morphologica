# -*- coding: utf-8 -*-

import h5py
import numpy as np
import os

# Parameters
dir = 'test' # This folder should exist. It should contain a config.json file
steps = 1000000
seed = 1
N = 5

# Define map (X-OR problem)
F = [0.0,1,1,0]
x = [0.0,0,1,1]
y = [0,1,0,1]
X = np.hstack([x,y])

h5f = h5py.File(dir+'/map.h5','w')
h5f.create_dataset('F', data=F)
h5f.create_dataset('X', data=X)
h5f.close()

# Define network (N nodes, fully recurrent)
pre = []
post = []
pre  = [0,0,1,1,2,3,4,4,4]
post = [2,3,2,3,3,2,2,3,4]
h5f = h5py.File(dir+'/network.h5','w')
h5f.create_dataset('pre', data=pre)
h5f.create_dataset('post', data=post)
h5f.close()

# Run the model
os.system('./build/recurrentnet '+dir+' '+str(seed)+' '+str(steps))

print("run './build/recurrentnet "+dir+" 0 -2' to see the results")



