'''
Created on 04.02.2018

@author: pli
'''
import os,sys
import itertools
import numpy as np
import soundfile
import scipy.signal
from paths import make_sure_path_exists
from tracking import compute_poaptdoa
sys.path.append("../python")
from config import WORKPATH, FILES



ARG_ALL = '--sampling-frequency 48000 --max-tau 14'

ARG_GAIN = {
'eg' :  "--gain-mode 3 --gain 40 --gain-smooth 0.05 --gain-max 24",
'cg' :  "--gain-mode 0 --gain 40 --gain-max 24",
}

ARG_CFG = { 
     '3a_m6' : "--bands 16 --fmin 300 --fmax 3000 --spike-mth 6 --spike-ath 0.0",    
    }

make_sure_path_exists(WORKPATH)

for ((cfg,cfg_arg),(gain,gain_arg)) in itertools.product(ARG_CFG.iteritems(),ARG_GAIN.iteritems()):
    cfgstr = '_'.join((cfg, gain))
    
    for (scenario, filepath) in FILES.iteritems():
        spikedpath = WORKPATH + scenario +'_'+ cfgstr + '.csv' 
        npypath = WORKPATH + "cor_"+ scenario +'_'+ cfgstr + '.npy' 
        res = compute_poaptdoa(filepath,
                         '--quiet ' +  ' '.join((ARG_ALL,cfg_arg,gain_arg))  ,
                         spikedpath, redo=False
                         )
        np.save(npypath, res)
        print npypath
                     
