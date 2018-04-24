'''
Created on 24.02.2018

@author: pli
'''

import sys,os
from numpy import fabs
from numba.types import none
sys.path.append('../python')
import rirgenerator as RG
import numpy as np
import soundfile
from config import GEO, AUDIOPATH, RIRPATH
import scipy.signal
from paths import make_sure_path_exists
import itertools

SPEEDOFSOUND = 343   # Sound velocity (m/s)
FS = 48000           # Sample frequency (samples/s)
NUMSAMPLES = 1<<15   # Number of samples            
ROOM         = np.array([6,5,5])     # Room dimensions [x y z] (m)
ROOM2        = np.array([5,4,3])     # Room dimensions [x y z] (m)
MIC_CENTERS  = np.array([
[ 2.18609094,  3.08879343,  1.60610754],
[ 3.28784210,  2.13980973,  1.25416981],
[ 2.64151881,  2.94940339,  1.66644353],
[ 3.34297847,  2.19302087,  1.35913821],
[ 2.34832611,  2.55705603,  1.67644908],
[ 2.34543316,  2.61919966,  1.55279962],
[ 3.13171514,  2.06510181,  1.66115169],
[ 3.05575952,  2.77312754,  1.70505226],
[ 3.90952129,  2.35478356,  1.41101156],
[ 3.21124699,  2.37349650,  1.36713422],
[ 3.50627389,  2.61377493,  1.54937366],
[ 3.46545925,  2.06965704,  1.64381688],
])
DISTS=[1.10610754,
0.75416981,
0.850645320243,
0.85913821,
1.17644908,
0.889592537327,
1.16115169,
1.20505226,
0.91101156,
0.86713422,
1.04937366,
1.05163759288]
    

make_sure_path_exists(RIRPATH)
make_sure_path_exists(AUDIOPATH)

for run,t60 in itertools.product(range(6),[0.3,0.45,0.6]):
    
    r = run + 30
    
    angles = range(0,360,5)
    np.random.shuffle(angles)
    for a in angles:
        filename = 'sim_c8_noise_t%03d_a%03d_r%02d.wav' % (int(t60*100),int(a),r)
        if (os.path.exists(AUDIOPATH+'/'+filename)):
            continue    
        rirfilename = RIRPATH + 'rir_c8_t%03d_a%03d_r%02d.npy' % (int(t60*100),int(a),r)             
        if os.path.exists(rirfilename):
            rirs = np.load(rirfilename)
            print 'loaded', rirfilename
        else:        
            s = MIC_CENTERS[run] + np.array([DISTS[run]*np.cos(a*np.pi/180.),DISTS[run]*np.sin(a*np.pi/180.),0.3])
            micpos = MIC_CENTERS[run] + GEO 
                                         
            print 'computing rirs'  , rirfilename
            rirs = RG.rir_generator(SPEEDOFSOUND, FS, micpos, s, ROOM, beta=t60, nsample=NUMSAMPLES, mtype='omnidirectional', hp_filter = 1)
            print rirs.shape
            np.save(rirfilename,rirs)
            
        SIGNAL = np.random.random((FS*2.2,))
        print 'convolving'
        out=[]
        for i in xrange(8):
            sig = scipy.signal.convolve(SIGNAL,rirs[i,:])
            out.append(sig[:FS*2])
        out = np.vstack(out).T
        print filename, out.shape
        soundfile.write(AUDIOPATH+'/'+filename,out,FS)
    print 
