'''
Created on 04.02.2018

@author: pli
'''


from config import *

import glob 
import sys,os
from rope.base.oi import doa
import numpy as np
sys.path.append("../python")
import evaldoas

tex=[]


for task in sorted(FILES.keys()):
    
    gt = evaldoas.load_gt_as_ta(DATAPATH+'groundtruth/'+task.replace('sq','')+"_gt.dat")
    tex.append('// '+task)    
    rr=[]
    
    results = sorted( glob.glob('./cnnoutput/*'+task+'*w20*.npy'))
    for result in results:  
        de = evaldoas.load_nhot_as_ta(result,6e-3,thres=0.5,roll=-18,collate=20)
        #print result        
        r = evaldoas.eval_ta(gt, de)
        (rmse,bias,f,pr,re) = r
        rr.append( ( f,evaldoas.string_result(*r)+ '  ' + os.path.basename(result) ) )
        if rmse is not None:
            tex.append(('%5.1f & %5.1f & %5.1f' % (rmse,pr,re)) + ' // ' +os.path.basename(result))
    
    print 
    print task
    print 
    ba = evaldoas.load_gt_as_ta(DATAPATH+'results/'+task.replace('sq','')+"qom6_dbc_sti.dat")
    r = evaldoas.eval_ta(gt, ba)    
    (rmse,bias,f,pr,re) = r
    tex.append('%5.1f & %5.1f & %5.1f // Baseline '  % (rmse,pr,re) )
    print evaldoas.string_result(*r) + '  IWAENC2012 paper'
    for (f, resstr) in sorted(rr, key = lambda (f,s) : f , reverse=True):
        print resstr
        
        
print
for line in tex:
    print line