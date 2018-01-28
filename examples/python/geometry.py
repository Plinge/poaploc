'''
Created on 28.01.2018

@author: pli
'''
import numpy as np

def radians(angle):
    return np.pi * angle / 180.0

def rotation_xy(rotationXY):
    r = radians(rotationXY)
    m = np.array( [np.cos(r),-np.sin(r), 0.0,
                   np.sin(r), np.cos(r), 0.0,
                         0.0,       0.0, 1.0] )
    m.shape = (3,3)
    return m
 
def circ_array(r,n=8,z=0.0):
    """ returns a list of mic positions for a circular array
        of radius r meters with n microphones
        z defines the height of every second microphone
    """
    ar = []
    for i in range(n):
        if i&1 == 1:
            v  = np.array([r, 0.0, z])
        else:
            v  = np.array([r, 0.0, 0.0])
        a = i * -360.0/float(n)
        ar.append(v.dot(rotation_xy(a)))
    s=[]
    for m in ar:
        s.append("%.3f,%.3f,%.3f" % (m[0],m[1],m[2]))
    return ','.join(s)