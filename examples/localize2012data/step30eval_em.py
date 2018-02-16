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
    evaldoas.print_result(*r)
    print 'ICASSP2012 paper'

    results = sorted(glob.glob(WORKPATH+'aem_'+task+"*_a0*.npy"))
    for result in results:
        de = evaldoas.load_em_as_ta(result)
        r = evaldoas.eval_ta_biased(gt, de)
        evaldoas.print_result(*r)
        print os.path.basename(result)
        