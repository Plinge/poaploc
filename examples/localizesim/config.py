'''
Created on 04.02.2018

@author: pli
'''
''' adust these paths as necessary '''
import os

EXTERNPATHBASE = "D:\\pli\\Experiments\\1804Loc\\"

if os.path.exists(EXTERNPATHBASE):
    SPEECHPATH  = EXTERNPATHBASE+'speech\\'
    WORKPATH  = EXTERNPATHBASE+'data\\'
    AUDIOPATH = EXTERNPATHBASE+'audio\\'
    RIRPATH   = EXTERNPATHBASE+'rirs/'
    CNNINPUTPATH   = EXTERNPATHBASE+'cnninput/'
else:
    SPEECHPATH  = './speech/'
    WORKPATH  = './data/'
    AUDIOPATH = './audio/'
    RIRPATH = './rirs/'
    CNNINPUTPATH = "./cnninput"

import tempfile
TEMPPATH = tempfile.gettempdir()+'/' # the current temporary directory

from geometry import circ_array, coordstring

GEO = circ_array(0.05,n=8)
ARG_GEO = '--mic-positions '+coordstring(GEO)

DOA_RESOLUTION = 5