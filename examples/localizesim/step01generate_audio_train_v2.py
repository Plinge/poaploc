'''
Created on 24.02.2018

@author: pli
'''

import sys,os
from numpy import fabs
from numba.types import none
import math
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

def wallfar(p,room):
    for dim in range(3):
        if p[dim]<0.5:
            return False
        if ROOM[dim]-p[dim]<0.5:
            return False
    return True

# mm=[];dd=[];hh=[]
# for _ in range(100):
#     
#     dist = 0.7 + np.random.random_sample() * 1.3
#     heig = 0.2 + np.random.random_sample() * 0.5
#     offset = np.array( [ np.random.random_sample() * 3.0 - 1.5, np.random.random_sample() * 3.0 - 1.5,  np.random.random_sample() * 0.5 - 0.25 ] )
#     m = ROOM2 / 2 + offset
#     if not wallfar(m, ROOM2):
#         continue
#     ok=True
#     for a in [0,90,180,270]:
#         oo = np.array([dist*np.cos(a*np.pi/180.),dist*np.sin(a*np.pi/180.),heig]) 
#         if not wallfar(m+oo, ROOM2):
#             ok=False
#         else:
#             print m+oo
#     if ok:
#         mm.append(m)
#         dd.append(dist)
#         hh.append(heig)
#         
#     if len(mm)>8: 
#         break
# print
# print mm
# print dd
# print hh
# sys.exit(0)


MIC_CENTERS  = np.array([
[ 2.18609094,  3.08879343,  1.60610754],
[ 1.80682374,  2.2414552 ,  1.2017824 ], 
[ 2.13622852,  2.4613022 ,  1.20128401], 
[ 2.46722745,  1.88017976,  0.99995121],
[ 1.68993685,  3.0249535 ,  0.80806865], 
[ 3.32456106,  1.44641776,  0.8616572 ], 
[ 1.35303387,  1.71907313,  1.18785852], 
[ 2.67672343,  3.11592682,  0.776473  ], 
[ 3.10837709,  2.43301665,  1.15264701], 
[ 2.01242434,  3.03967996,  0.78881824]])


DISTS=[1.1061,1.0131059448856545, 1.5651661941536754, 0.9916259051440476, 1.0315978429771195, 0.9177406464822381, 0.7125837059765597, 1.1091980278315332, 0.7019514491870887, 1.130039765076737]

HEIHGTS=[0.3,0.5603202560038713, 0.49757306794340384, 0.30131776796435195, 0.21216374071023852, 0.4311553149797613, 0.6820993924916054, 0.29903433242065425, 0.38229889633530206, 0.24935086099707432]
        
    
make_sure_path_exists(RIRPATH)
make_sure_path_exists(AUDIOPATH)

for run,t60 in itertools.product(range(10),[0.3,0.45,0.6]):
    
    r = run + 30
    room = ROOM if r<5 else ROOM2 
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
            mic_center = MIC_CENTERS[run]  
            s = mic_center + np.array([DISTS[run]*np.cos(a*np.pi/180.),DISTS[run]*np.sin(a*np.pi/180.),HEIHGTS[run]])
            
            if not wallfar(mic_center,room):
                raise Exception('microphones too close to the wall')
            if not wallfar(s,room):
                raise Exception('speaker too close to the wall')
                                
            micpos = MIC_CENTERS[run] + GEO 
            m = np.mean(micpos,0)
            print 'source', np.round(s,2),'dist',np.round(DISTS[run],2),  'mic', np.round (m,2),'elevation %.1f' % (180.0/np.pi * math.atan2(s[2]-m[2],math.pow(s[0]-m[0],2) +math.pow(s[1]-m[1],2) ))
                                         
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
