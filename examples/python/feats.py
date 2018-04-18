'''
Created on 18.04.2018

@author: pli
'''
import numpy as np

def running_mean(x, N):
    if N<2:
        return x
    cumsum = np.cumsum(x, 0) 
    return (cumsum[N:] - cumsum[:-N]) / float(N)

def spikenorm(xx):
    xx = np.sqrt(xx*1.0/128.0)
    ma = xx.max() 
    xx[xx>1.0]=1.0
    return xx, ma         