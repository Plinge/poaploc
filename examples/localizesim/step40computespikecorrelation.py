'''
Created on 05.04.2018

@author: pli
'''
import os,sys,glob
import itertools
import numpy as np
import soundfile
import scipy.signal
from paths import make_sure_path_exists
from tracking import compute_poaptdoa
sys.path.append("../python")
from config import WORKPATH, TEMPPATH, AUDIOPATH

REDO = False

ARG_ALL = '--sampling-frequency 48000 --max-tau 14 --start-time 0.02 --end-time 2.0'

ARG_GAIN = {
#'cg' :  "--gain-mode 0 --gain 40",
'dg' :  "--gain-mode 0 --gain 0",
#'eg' :  "--gain-mode 3 --gain 40 --gain-smooth 0.05 --gain-max 24",
'fg' :  "--gain-mode 3 --gain 0 --gain-smooth 0.05 --gain-max 24",
}

ARG_CFG = { 
     '3a_m6' : "--bands 16 --fmin 300 --fmax 3000 --spike-mth 6 --spike-ath 0.0",    
 #    '3a_m0' : "--bands 16 --fmin 300 --fmax 3000 --spike-mth 0 --spike-ath 0.0",
    }

make_sure_path_exists(WORKPATH)

for ((cfg,cfg_arg),(gain,gain_arg)) in itertools.product(ARG_CFG.iteritems(),ARG_GAIN.iteritems()):
    cfgstr = '_'.join((cfg, gain))

    for (filepath) in glob.glob(AUDIOPATH+'\*.wav'):
        scenario = os.path.basename(filepath).replace('.wav','')
        npypath = WORKPATH + "cor_"+ scenario +'_'+ cfgstr + '.npy' 
        if (os.path.exists(npypath)) and not REDO:
            continue
        spikedpath = TEMPPATH + scenario +'_'+ cfgstr + '.csv' 
        res = compute_poaptdoa(filepath,
                         '--quiet ' +  ' '.join((ARG_ALL,cfg_arg,gain_arg))  ,
                         spikedpath, skip=64,  redo=REDO)                
        np.save(npypath, res)
        os.unlink(spikedpath)
        print npypath, res.shape, res.min(), '..', res.max()
