'''
Created on 04.02.2018

@author: pli
'''
''' adust these paths as necessary '''

WORKPATH  = './data/'
AUDIOPATH = './audio'

import tempfile
TEMPPATH = tempfile.gettempdir()+'/' # the current temporary directory

from geometry import circ_array, coordstring

GEO = circ_array(0.05,n=8)
ARG_GEO = '--mic-positions '+coordstring(GEO)

DOA_RESOLUTION = 5