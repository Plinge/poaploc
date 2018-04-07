'''
Created on 04.02.2018

@author: pli
'''
import os,sys
import itertools
import numpy as np
import glob
import evaldoas
from paths import make_sure_path_exists
from tracking import compute_poapmloc
sys.path.append("../python")
from config import WORKPATH, TEMPPATH, FILES, ARG_GEO

ARG_ALL = '--precision 1 --elevation-precision 5 --min-likelihood 0.01 --skip 3 \
--post-min-bands 4 --post-min-energy=-40 \
--sampling-frequency 48000 \
--maxelevation --max-elevation 45 \
--frame-length 12 --frame-step 6 \
--post-min-bands 4 --post-min-energy=-40'

ARG_MODE = {
    'poa' : '',
#    'poap' : '--poap',
    }

ARG_GAIN = {
'cg' :  "--gain-mode 0 --gain 40 --gain-max 24",
'dg' :  "--gain-mode 0 --gain 0 --gain-max 24",
'eg' :  "--gain-mode 3 --gain 40 --gain-smooth 0.05 --gain-max 24",
'fg' :  "--gain-mode 3 --gain 0 --gain-smooth 0.05 --gain-max 24",
}

ARG_CFG = { 
#    '3a_g05_m6' : "--bands 16 --fmin 300 --fmax 3000 --gamma 0.5 --spike-mth 6 --spike-ath 0.0",
    '3a_g03_m6' : "--bands 16 --fmin 300 --fmax 3000 --gamma 0.3 --spike-mth 6 --spike-ath 0.0",    
#    '6a_g03_m6' : "--bands 24 --fmin 300 --fmax 6000 --gamma 0.3 --spike-mth 6 --spike-ath 0.0",    
    }

ARG_POST = {
   '5' : '--post-time 0.5',
#   '1' : '--post-time 0.1',
#    '0' : '',
            }

make_sure_path_exists(WORKPATH)

for ((mode,mode_arg),(cfg,cfg_arg),(gain,gain_arg),(post,post_arg)) in itertools.product(ARG_MODE.iteritems(),ARG_CFG.iteritems(),ARG_GAIN.iteritems(),ARG_POST.iteritems()):
    cfgstr = '_'.join((mode, cfg, gain, post))
    
    for (scenario, filepath) in FILES.iteritems():
     
        compute_poapmloc(filepath,
                         '--quiet ' +  ' '.join((ARG_ALL,ARG_GEO,mode_arg,cfg_arg,gain_arg,post_arg))  ,
                         'msl_' + scenario +'_'+ cfgstr,
                         TEMPPATH + scenario +'_'+ cfgstr + '.csv' ,
                         WORKPATH, redo=False)
