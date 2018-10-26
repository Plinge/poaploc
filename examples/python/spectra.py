# -*- coding: utf-8 -*-
'''
Created on 21 Feb 2013

@author: aplinge
'''
import math
import numpy as np

def specstring(xx):
    s=''
    for x in xx:
        x=x*100.0
        if x>99.0:
            s = s + ' ##'
        elif x>1.0:
            s = s + ("%3d" % int(x))
        else:
            s = s + '  -'
    return s
    
def energy(x):    
    if x<=0.0:
        return -120.0
    en= 10.0*math.log(x)    
    en = (40.0+en)/60.0
    if en<0.0:
        en = 0.0
    if en>1.0:
        en = 1.0
        
def normalize(s):
    slen = math.sqrt(np.dot(s,s.T))
    s = np.dot(s,1.0/slen)
    return s

def correlation_n(s,t):
    if s.shape != t.shape:
        print ("shape mismatch",  s.shape, " ", t.shape)
        exit(2)
    corr = np.dot(s,t.T)
    return corr


def correlation(s,t):
    if s.shape != t.shape:
        print ("shape mismatch",  s.shape, " ", t.shape)
        exit(2)
    s = normalize(s)    
    t = normalize(t)
    corr = np.dot(s,t.T)
    return corr

def correlation1(s,t):
    if s.shape != t.shape:
        print  ("shape mismatch",  s.shape, " ", t.shape)
        exit(2)
    corr = 0
    slen = 0
    tlen = 0            
    for i in range(0,len(s)):
        corr = corr + s[i]*t[i]
        slen = slen + s[i]*s[i]
        tlen = tlen + t[i]*t[i]
    corr = corr / (math.sqrt(slen+tlen)*len(s))
    return corr
        
def weightedAverage2(l,w):
    import itertools
    s = None 
    n = 0.0
    for (x,p) in itertools.izip(l,w):
        if s == None:
            s = np.zeros((len(x),1))
            s.shape = (s.size)
        #su = su + np.sum(x)
        t = np.dot(x,p)
        s = s + t
        n = n + p        
    su =  np.sum(s)     
    if n>0.0:
        s = np.dot(s,1.0/n)
    s.shape=(s.size)
    return [s, su] 
        
        
def weightedAverage(l,w):    
    return weightedAverage2(l, w)[0]
