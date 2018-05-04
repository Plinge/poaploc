'''
Created on 04.02.2018

@author: pli
'''

from config import *
 
import sys
sys.path.append("../python")

from tracking import compute_em

for task in FILES.keys():
    #compute_em(WORKPATH, num_arrays=1,pattern=task+'_poa_*g_5',redo=True,win=2.0)
    compute_em(WORKPATH, num_arrays=1,pattern=task+'_poa_*g_5',redo=True,win=1.0)
    #compute_em(WORKPATH, num_arrays=1,pattern=task+'_poa_*fg_5',redo=True,win=0.1)