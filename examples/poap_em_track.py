'''
Created on 28.01.2018

@author: pli
'''

DATAPATH = "C:/data/iwaenc2012ast_dortmund/data/"
WORKPATH  = './data/'
FILES = {
'fi11sq01' : DATAPATH + 'fi11sq01-static_1sp-array.wav',
'fi11sq04' : DATAPATH + 'fi11sq04_1sp_1mp-array.wav',
'fi11sq06' : DATAPATH + 'fi11sq06-2mp_cross-array.wav', 
'fi13sq02' : DATAPATH + 'fi13sq02_2mp-array.wav',
      }


import sys
sys.path.append("./python")
from tracking import compute_em, compute_msx
import numpy as np

from geometry import circ_array

ARGS = '--mic-positions '+circ_array(0.05)

compute_msx(FILES, ARGS, WORKPATH,'msl',False)
compute_em(WORKPATH, num_arrays=1,pattern='fi')