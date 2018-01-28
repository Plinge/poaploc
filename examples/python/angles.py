# -*- coding: utf-8 -*-
'''
Created on 21 Feb 2013

@author: aplinge
'''


import math
import numpy as np
import itertools
import random

"""
some math for angles
"""
def make360(x):    
    while x < 0.0:
        x = x+360.0
    while x > 360.0:
        x = x-360.0        
    return x
    
def make180(x):
    while x > 180.0:
        x = x-360.0
    while x < -180.0:
        x = x+360.0
    return x

def difference(a,b):
    return abs(make180(abs(a-b)))

def differences(a,b):
    return make180(a-b)

def goodness(a0,a1):
    """
    qualitiy of localization by intersection
    """
    return 1.0-(abs(90.0-abs(make180(a0-a1)))/90.0)

def average(a):
    import itertools
    mi, ma =  None, None
    n  = 0
    a,a2 = itertools.tee(a)
    for x in a2:
        if mi == None or x < mi:
            mi = x
        if ma == None or x > ma:
            ma = x
        n=n+1
    if n<1:
        print "list is empty"
        return 0.0
    s = 0.0
    if (mi < -90 and ma > 90):
        for x in a:
            s = s + make180(x + 180.0)
        s = s / n - 180.0
    else:
        s = 0.0
        for x in a:
            s = s + make180(x)
        s = s / n
    #print "av " , a, " = ",s
    return s

def weightedAverage(a,p):
    """
    average angle weighted
    """
    import itertools
    mi, ma =  None, None
    n  = 0
    a,a2 = itertools.tee(a)
    for x in a2:
        if mi == None or x<mi:
            mi = x
        if ma == None or x>ma:
            ma = x
        n=n+1
    if n<1:
        print "list is empty"
        return 0.0
    s = 0.0
    n = 0.0
    if (mi < -90 and ma > 90):        
        for x,w in itertools.izip(a,p):
            xx = make180(x + 180.0)
            s = s + xx*w
            n = n + w
        if n>0:
            s = make180((s / n) - 180.0)
    else:        
        s = 0.0
        for x,w in itertools.izip(a,p):            
            s = s + make180(x)*w
            n = n + w
        if n>0:
            s =  make180(s / n)
    return s

    
class NormalDevAngles:
    def __init__(self,m=None,v=10.0):
        if m != None:
            self.set(m,v)
       
    def logprob(self,x):
        d = difference(x, self.mu)
        d = math.log(d*d) + 2.0*self.logva + self.logde
        return -d
    
    def prob(self,x):
        d = difference(x, self.mu)
        d = math.exp(- d*d / (2.0*self.var))
        d = d / self.denom
        return d
         
    def getmu(self):
        return self.mu

    def getsigma(self):
        return math.sqrt(self.var)

    def getvar(self):
        return self.var

    mu = property(getmu,None,None,"median")
    sigma = property(getsigma,None,None,"sigma")
    var = property(getvar,None,None,"variance")
    
    def set(self,m,v):
        self.mu = m
        # hard limit for variance
        if v<=0.5:
            v = 0.5
        self.var = v
        self.logva = math.log(self.var)
        self.denom = math.sqrt(v*np.pi*2.0)
        self.logde = math.log(self.denom)

    def update(self,a,w=None):
        """
        update distribution
        @param a  list of angles
        @param w  weight per angle, if not given  all angles are treated equal
        """
        import itertools
        if w:
            a,a2 = itertools.tee(a)
            w,w2 = itertools.tee(w)
            s = weightedAverage(a2,w2)
        else:
            a,a2 = itertools.tee(a)
            s = average(a2)
        v = 0.0
        if w:
            n = 0.0
            for x,p in itertools.izip(a,w):
                v = v + p * math.pow(difference(x, s), 2.0)
                n = n + p
            if n>0:
                v = v / n
            else:
                print "zero zum"
        else:
            for x in a:
                v = v + math.pow(difference(x, s), 2.0)
            v = v / len(a)
        self.set(s,v)

    def update_variance(self,a,w=None):
        """
        update variance only
        @param a  list of angles
        @param w  weight per angle, if not given all angles are treated equal
        """
    
        v = 0.0
        if w:
            n = 0.0
            for x,p in itertools.izip(a,w):
                v = v + p * math.pow(difference(x, self.mu), 2.0)
                n = n + p
            if n>0:
                v = v / n
            else:
                print "zero zum"
        else:
            for x in a:
                v = v + math.pow(difference(x, self.mu), 2.0)
            v = v / len(a)
        self.set(self.mu,v)


if __name__ == '__main__':
    
    for _ in xrange(10):
        a = random.uniform(-720,720)
        b = random.uniform(-720,720)
        x = make360(a-b+180) - 180 
        y = make180(a-b)
        print a,b, x,y
        