'''
Created on 04.02.2018

@author: pli
'''
''' adust these paths as necessary '''
DATAPATH = "C:/data/iwaenc2012ast_dortmund/" 
WORKPATH  = './data/'

AUDIOPATH = DATAPATH+"data/"

FILES = {
'fi11sq01' : AUDIOPATH + 'fi11sq01-static_1sp-array.wav',
'fi11sq04' : AUDIOPATH + 'fi11sq04_1sp_1mp-array.wav',
'fi11sq06' : AUDIOPATH + 'fi11sq06-2mp_cross-array.wav', 
'fi13sq02' : AUDIOPATH + 'fi13sq02_2mp-array.wav',
      }

from geometry import circ_array

ARG_GEO = '--mic-positions '+circ_array(0.05)
