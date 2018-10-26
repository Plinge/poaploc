'''
Created on 04.02.2018

@author: pli
'''
from __future__ import print_function
import os,sys
import itertools
import numpy as np
import glob

sys.path.append("../python")

import evaldoas
from paths import make_sure_path_exists
from tracking import compute_poapmloc
from config import WORKPATH, TEMPPATH, AUDIOPATH, ARG_GEO

ARG_ALL = '--precision 1 --elevation-precision 5 --min-likelihood 0.01 --skip 3 \
--post-min-energy=-40 \
--sampling-frequency 48000 \
--maxelevation --max-elevation 45 \
--frame-length 12 --frame-step 6 \
--post-min-bands 4 --post-min-energy=-40'

ARG_MODE = {
#    'poa' : '',
#    'poap' : '--poap',
    'max' : '--poap --sum-bands --argmax',
}

ARG_GAIN = {
'cg' :  "--gain-mode 0 --gain 40 --gain-max 24",
#'dg' :  "--gain-mode 0 --gain 0 --gain-max 24",
#'eg' :  "--gain-mode 3 --gain 40 --gain-smooth 0.05 --gain-max 24",
'fg' :  "--gain-mode 3 --gain 0 --gain-smooth 0.05 --gain-max 24",
}

ARG_CFG = { 
    '04b3_g03_m6' : "--bands  4 --post-min-bands 1 --fmin 300 --fmax 3000 --gamma 0.3 --spike-mth 6 --spike-ath 0.0",
    '08b3_g03_m6' : "--bands  8 --post-min-bands 2 --fmin 300 --fmax 3000 --gamma 0.3 --spike-mth 6 --spike-ath 0.0",    
    '16b3_g03_m6' : "--bands 16 --post-min-bands 4 --fmin 300 --fmax 3000 --gamma 0.3 --spike-mth 6 --spike-ath 0.0",    
    '32b3_g03_m6' : "--bands 32 --post-min-bands 8 --fmin 300 --fmax 3000 --gamma 0.3 --spike-mth 6 --spike-ath 0.0",
    #'16b6_g03_m6' : "--bands 16 --post-min-bands 4 --fmin 300 --fmax 6000 --gamma 0.3 --spike-mth 6 --spike-ath 0.0",    
    #'32b6_g03_m6' : "--bands 32 --post-min-bands 8 --fmin 300 --fmax 6000 --gamma 0.3 --spike-mth 6 --spike-ath 0.0",  
    '16b3_g03_zc' : "--bands 16 --post-min-bands 4 --fmin 300 --fmax 3000 --gamma 0.3 --spike-mode 3 --spike-ath 0.0",    
}

ARG_POST = {
#   '5' : '--post-time 0.5',
#   '1' : '--post-time 0.1',
    '0' : '',
}

make_sure_path_exists(WORKPATH)
outpath = WORKPATH + '/speechmax/'
make_sure_path_exists(outpath)

for ((mode,mode_arg),(cfg,cfg_arg),(gain,gain_arg),(post,post_arg)) in itertools.product(ARG_MODE.items(),ARG_CFG.items(),ARG_GAIN.items(),ARG_POST.items()):
    cfgstr = '_'.join((mode, cfg, gain, post))
    prefix = 'msl'
    if 'argmax' in mode_arg:
        prefix = 'msn'    
    filelist = glob.glob(AUDIOPATH+'/sim_c8_speech_*.wav')
    for fileindex,filepath in enumerate(filelist):
        print (1+fileindex,'/',len(filelist)),
        scenario = os.path.basename(filepath.split('.')[-2])
        compute_poapmloc(filepath,
                         '--quiet ' + ' '.join((ARG_ALL,ARG_GEO,mode_arg,cfg_arg,gain_arg,post_arg)),
                         prefix + '_' + scenario +'_'+ cfgstr,
                         TEMPPATH + scenario +'_'+ cfgstr + '.csv' ,
                         outpath, redo=False)
