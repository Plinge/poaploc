'''
Created on 04.02.2018

@author: pli
'''
import itertools
import numpy as np
import angles
import pandas as pd

def _get_fpr(tp,fp,fn):
    f,pr,re=0.0,0.0,0.0
    if tp+fp>0:
        pr = 100.0*tp/float(tp+fp)
    if tp+fn>0:
        re = 100.0*tp/float(tp+fn)
    if pr+re>0.0:
        f = (2*pr*re)/(pr+re)
    return f, pr, re

def angles_min_dist_permutation(list1,list2):    
    l1 = len(list1);l2 = len(list2)    
    if l1>l2:
        temp=list2;list2=list1;list1=temp
        l1=len(list1);l2=len(list2)
    ''' l1 is the shorter list '''
    midi = None
    best = None
    for list2per in itertools.permutations(list2):
        dists=[]
        for (a1,a2) in zip(list1,list2per[:l1]):
            dists.append( angles.differences(a1,a2) )
        dist = np.sqrt(np.sum((v*v for v in dists)))
        if midi is None or dist<midi:
            midi=dist
            best=[midi,dists,list1,list2per]            
    return best

def load_gt_as_ta(filename):
    data = pd.read_csv(filename,sep='\s',engine='python')
    len  = data.time.size
    res={}
    for index in xrange(len):
        t = data.time[index]
        if not t in res.keys():
            res[t]=[]
        res[t].append(-data.angle[index])
    del data
    return res

def load_gt_as_framed(filename, framestep):
    data = pd.read_csv(filename,sep='\s',engine='python')
    frames = int( np.ceil( data.time.max() / float(framestep) ) )
    res=np.ones((frames,4)) * -999
    for index in xrange(data.time.size):
        t = data.time[index]
        frameindex = int ( t/ float(framestep) ) 
        res[ frameindex, data.person[index]-1 ] = -data.angle[index]
    del data
    return res
    
def load_em_as_ta(filename):
    data = np.load(filename)
    res={}
    for index in xrange(data.shape[0]):
        t = data[index,1]
        if not t in res.keys():
            res[t]=[]
        res[t].append(data[index,2])
    del data
    return res

def add_bias_ta(tadata,bias):
    res={}
    for t in tadata.keys():
        res[t] = np.array(tadata[t]) + bias
    return res

def eval_ta(gt,de,window=0.13,maxdoaerror=15):
    gt_times = sorted(gt.keys())
    de_times = np.array(sorted(de.keys()))
    fn, fp, tn, tp = 0,0,0,0
    errors = []
    for time in gt_times:
        gt_doas = np.array(gt[time])
        i = np.argmin(np.abs(de_times-time))
        de_time = de_times[i]
        if np.abs(de_time-time)>window:
            fn += len(gt_doas)            
            continue
        de_doas = np.array(de[de_time])
        [midi,dists,list1,list2per] = angles_min_dist_permutation(gt_doas, de_doas)
        errors.extend(dists)
        tt = len(filter(lambda x:x<=maxdoaerror, dists))
        tp = tp + tt
        fn = fn + len(gt_doas)-tt
        if len(de_doas)>len(gt_doas):
            fp = fp + len(de_doas)-len(gt_doas)
    rmse = np.sqrt(np.sum(v*v for v in errors)/float(len(errors)))
    bias = np.mean(errors)
    f,pr,re=_get_fpr(tp,fp,fn)
    return rmse,bias,f,pr,re
    
def eval_ta_biased(gt,de,window=0.13,maxdoaerror=15):
    _,bias,_,_,_ = eval_ta(gt,de,window,maxdoaerror)
    de = add_bias_ta(de, bias)
    rmse,_,f,pr,re = eval_ta(gt,de,window,maxdoaerror)
    return rmse,bias,f,pr,re

def string_result(rmse,bias,f,pr,re):    
    return ("RMSE %5.1f(%5.1f) deg "%(rmse,bias)) + ( "%6.1f%% pr %6.1f%% re %6.1f%%"%(pr,re,f))

    
def print_result(rmse,bias,f,pr,re):    
    print string_result
                
        