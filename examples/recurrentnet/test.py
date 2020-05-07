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
seed = 3
N = 5

# Define map (X-OR problem)
h5f = h5py.File(dir+'/map.h5','w')
h5f.create_dataset('X', data=[0,0,1,1])
h5f.create_dataset('Y', data=[0,1,0,1])
h5f.create_dataset('Z', data=np.zeros(4))
h5f.create_dataset('F', data=[0,1,1,0])
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

print("run './build/recurrentnet "+dir+" 0 -1' to see the results")


