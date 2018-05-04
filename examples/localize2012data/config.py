'''
Created on 04.02.2018

@author: pli
'''
''' adust these paths as necessary '''
DATAPATH = "../../../iwaenc2012ast_dortmund/" 
WORKPATH  = './data/'
AUDIOPATH = DATAPATH+"data/"

FILES = {
'fi11sq01' : AUDIOPATH + 'fi11sq01-static_1sp-array.wav',
'fi11sq04' : AUDIOPATH + 'fi11sq04_1sp_1mp-array.wav',
'fi11sq06' : AUDIOPATH + 'fi11sq06-2mp_cross-array.wav', 
'fi13sq02' : AUDIOPATH + 'fi13sq02_2mp-array.wav',
      }

import tempfile
TEMPPATH = tempfile.gettempdir()+'/' # the current temporary directory

from geometry import circ_array, coordstring

GEO = circ_array(0.05,n=8)
ARG_GEO = '--mic-positions '+coordstring(GEO)
DOA_RESOLUTION = 5