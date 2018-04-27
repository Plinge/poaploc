'''
Created on 05.04.2018

@author: pli
'''
import os,sys,glob
import itertools
import numpy as np
from paths import make_sure_path_exists
from tracking import compute_poaptdoa
sys.path.append("../python")
from config import WORKPATH, TEMPPATH, AUDIOPATH

REDO = False

ARG_ALL = '--sampling-frequency 48000 --max-tau 14 --start-time 0.02'

ARG_GAIN = {
#'eg' :  "--gain-mode 3 --gain 40 --gain-smooth 0.05 --gain-max 24",
'fg' :  "--gain-mode 3 --gain 0 --gain-smooth 0.05 --gain-max 24",
'dg' :  "--gain-mode 0 --gain 0",
#'cg' :  "--gain-mode 0 --gain 40",
}

ARG_CFG = { 
     '3a_m6' : "--bands 16 --fmin 300 --fmax 3000 --spike-mth 6 --spike-ath 0.0",    
 #    '3a_m0' : "--bands 16 --fmin 300 --fmax 3000 --spike-mth 0 --spike-ath 0.0",
    }

make_sure_path_exists(WORKPATH)
ALLWAVS = glob.glob(AUDIOPATH+'\*.wav')
#np.random.shuffle(ALLWAVS)
print len(ALLWAVS),'wav files'
for (fileindex,filepath) in enumerate(ALLWAVS):
    for ((cfg,cfg_arg),(gain,gain_arg)) in itertools.product(ARG_CFG.iteritems(),ARG_GAIN.iteritems()):
        cfgstr = '_'.join((cfg, gain))
        scenario = os.path.basename(filepath).replace('.wav','')
        npypath = WORKPATH + "cor_"+ scenario +'_'+ cfgstr + '.npy' 
        if (os.path.exists(npypath)) and not REDO:
            continue
        print fileindex+1,'/',len(ALLWAVS),('  %.2f%%' % ((fileindex+1.0)*100.0/len(ALLWAVS))) 
        spikedpath = TEMPPATH + scenario +'_'+ cfgstr + '.csv' 
        res = compute_poaptdoa(filepath,
                         '--quiet ' +  ' '.join((ARG_ALL,cfg_arg,gain_arg))  ,
                         spikedpath, skip=64,  redo=True)                
        np.save(npypath, res)
        os.unlink(spikedpath)
        print npypath, res.shape, res.min(), '..', res.max()
