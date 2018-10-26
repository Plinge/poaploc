'''
Created on 04.02.2018

@author: pli
'''
from __future__ import print_function
import numpy as np
import glob
import sys,os
sys.path.append("../python")
from config import WORKPATH
from evaldoas import eval_static_doas  


def eval(pattern):

    results = {}
     
    files = sorted(glob.glob(WORKPATH+ '/speechmax/'+'msn'+pattern+'.npy'))
    #np.random.shuffle(files)
    for filepath in files:
        filename = os.path.basename(filepath).split('.')[-2]
        vals = filename.split('_')
        #print vals
        scenario = vals[4:7]
        #print scenario
            
        t60 = int(vals[4][1:])/100.0 
        gtdoa = int(vals[5][1:])+90
        config = vals[7:-1]
        window = int(config[-1])
        #print config
        
        if window>0:
            continue
         
        adata = np.load(filepath)
        #print adata[1,:]
        #print sceneparam[2], np.mean(adata[:,2])
        stats = eval_static_doas(adata, None, [ gtdoa, ] )
        print (scenario,  config, ('%6.2f %5.2f%%'% (stats['err'], stats['re'])))
    #    break 
        configkey = '-'.join(config)
        if configkey not in results.keys():
            results[configkey]={}
        rescfg = results[configkey]
        
        if t60 in rescfg:
            rescfg[t60].append(stats['err'])
        else:
            rescfg[t60] = [stats['err'], ]
            
    for configkey in sorted(results.keys()):    
        s = configkey
        rescfg = results[configkey]
        for t60 in sorted(rescfg.keys()):
            s = s + ('%6.2f' % np.mean(np.abs(rescfg[t60])))            
        print(s)
    print
            
#eval('*_m6*_cg*')

eval('*16b3*_cg*')
