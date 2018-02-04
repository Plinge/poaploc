'''
Created on 28.01.2018

@author: pli
'''

''' adust these paths as necessary '''
DATAPATH = "C:/data/iwaenc2012ast_dortmund/" 
WORKPATH  = './data/'
REDO = False

AUDIOPATH = DATAPATH+"data/"

FILES = {
'fi11sq01' : AUDIOPATH + 'fi11sq01-static_1sp-array.wav',
'fi11sq04' : AUDIOPATH + 'fi11sq04_1sp_1mp-array.wav',
'fi11sq06' : AUDIOPATH + 'fi11sq06-2mp_cross-array.wav', 
'fi13sq02' : AUDIOPATH + 'fi13sq02_2mp-array.wav',
      }


import os,sys
sys.path.append("./python")
from tracking import compute_em, compute_msx
import numpy as np
import glob
import evaldoas

from geometry import circ_array

ARGS = '--mic-positions '+circ_array(0.05)

compute_msx(FILES, ARGS, WORKPATH,'msl',redo=REDO)
for task in FILES.keys():
    compute_em(WORKPATH, num_arrays=1,pattern=task,redo=REDO)
for task in sorted(FILES.keys()):
    gt = evaldoas.load_gt_as_ta(DATAPATH+'groundtruth/'+task.replace('sq','')+"_gt.dat")
    results = glob.glob(WORKPATH+'aem_'+task+"*_a0.npy")
    for result in results:
        de = evaldoas.load_em_as_ta(result)
        print os.path.basename(result),
        evaldoas.eval_ta(gt, de)
        
     