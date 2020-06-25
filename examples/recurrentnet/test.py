# -*- coding: utf-8 -*-

import h5py
import numpy as np
import os

def recur(pre, post, inds, selfconns=False):
    for i in inds:
        for j in inds:
            if (selfconns or ~(i==j)):
                pre = np.hstack([pre,i])
                post = np.hstack([post,j])
    return pre, post


# Parameters
dir = 'test' # This folder should exist. It should contain a config.json file
steps = 10000
seed = 1
N = 5

# Define map (X-OR problem)
F = [0,1,1,0]
x = [0,0,1,1]
y = [0,1,0,1]
X = np.hstack([x,y])

h5f = h5py.File(dir+'/map.h5','w')
h5f.create_dataset('F', data=F)
h5f.create_dataset('X', data=X)
h5f.close()

# Define network (N nodes, fully recurrent)
pre = []
post = []
pre, post = recur(pre,post,np.arange(N))
h5f = h5py.File(dir+'/network.h5','w')
h5f.create_dataset('pre', data=pre)
h5f.create_dataset('post', data=post)
h5f.close()

# Run the model
os.system('./build/recurrentnet '+dir+' '+str(seed)+' '+str(steps))

print("run './build/recurrentnet "+dir+" 0 -2' to see the results")



