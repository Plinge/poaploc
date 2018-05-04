'''
Created on 04.02.2018

@author: pli
'''


from config import *

import glob 
import sys,os

sys.path.append("../python")
import evaldoas

for task in sorted(FILES.keys()):
    print 
    print task
    print 
    gt = evaldoas.load_gt_as_ta(DATAPATH+'groundtruth/'+task.replace('sq','')+"_gt.dat")

    ba = evaldoas.load_gt_as_ta(DATAPATH+'results/'+task.replace('sq','')+"qom6_dbc_sti.dat")
    r = evaldoas.eval_ta_biased(gt, ba)
    
    print evaldoas.string_result(*r), 'ICASSP2012 paper'

    results = sorted(glob.glob(WORKPATH+'aem_'+task+"*m6_fg_*a0*.npy"))
    rr=[]
    for result in results:
        de = evaldoas.load_em_as_ta(result)
        if len(de)<2:
            continue
        r = evaldoas.eval_ta_biased(gt, de)
        (rmse,bias,f,pr,re) = r
        rr.append( ( f,evaldoas.string_result(*r)+ '  ' + os.path.basename(result) ) )
        
        
    
    for (f, resstr) in sorted(rr, key = lambda (f,s) : f , reverse=True):
        print resstr