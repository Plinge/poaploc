'''
Created on 24.02.2018

@author: pli
'''

import sys,os
from numpy import fabs
from numba.types import none
import math, time
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
ROOM3        = np.array([4,4,2.5])     # Room dimensions [x y z] (m)
ROOM4        = np.array([4,5,2.5])     # Room dimensions [x y z] (m)

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
#     if not wallfar(m, ROOM3):
#         continue
#     ok=True
#     for a in [0,90,180,270]:
#         oo = np.array([dist*np.cos(a*np.pi/180.),dist*np.sin(a*np.pi/180.),heig]) 
#         if not wallfar(m+oo, ROOM2):
#             ok=False
#         #else:
#         #    print m+oo
#     if ok:
#         mm.append(m)
#         dd.append(dist)
#         hh.append(heig)
#          
#     if len(dd)>=10: 
#         break
# print
# print np.round(mm,4)
# print np.round(dd,4)
# print np.round(hh,4)
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
[ 2.01242434,  3.03967996,  0.78881824],
[ 2.6763,  2.5782,  0.87  ],
 [ 2.1645,  2.857,   1.0528],
 [ 2.6665,  2.3918,  0.9609],
 [ 2.1313,  2.3886,  0.996 ],
 [ 2.3173,  2.4522,  1.2018],
 [ 1.7039,  2.7321,  1.1952],
 [ 1.5875,  2.4951,  1.1119],
 [ 1.8936,  1.744,   1.1555],
 [ 3.0402,  3.1484,  0.9855],
 [ 3.1401,  2.2977,  0.9636],
])


DISTS=[1.1061,1.0131059448856545, 1.5651661941536754, 0.9916259051440476, 1.0315978429771195, 0.9177406464822381, 0.7125837059765597, 1.1091980278315332, 0.7019514491870887, 1.130039765076737,
       0.7852,  1.27,    1.0714,  0.9997,  1.2352,  0.9782,  0.8549,  0.8323,  0.8526,  1.0407 ]

HEIHGTS=[0.3,0.5603202560038713, 0.49757306794340384, 0.30131776796435195, 0.21216374071023852, 0.4311553149797613, 0.6820993924916054, 0.29903433242065425, 0.38229889633530206, 0.24935086099707432,
         0.2577,  0.6506,  0.6907,  0.279,   0.4007,  0.3054,  0.2039,  0.5094,  0.2722,0.5896]
        
    
make_sure_path_exists(RIRPATH)
make_sure_path_exists(AUDIOPATH)

for run,t60 in itertools.product([0],[0.45,0.15,0.3,0.6]):
    
    r = run + 55
    room = ROOM
    mic_center = MIC_CENTERS[run]
    dist = DISTS[run]  
    rirsamples = min( NUMSAMPLES, t60*FS)
    angles = range(0,360,5)
    np.random.shuffle(angles)
    elevations=range(0,35,5)
    for a,e in itertools.product(angles,elevations):

        filename = 'sim_c8_noise_t%03d_a%03d_e%02d_r%02d.wav' % (int(t60*100),int(a),int(e),r)
        if (os.path.exists(AUDIOPATH+'/'+filename)):
            continue    
        rirfilename = RIRPATH + 'rir_c8_t%03d_a%03d_e%02d_r%02d.npy' % (int(t60*100),int(a),int(e),r)             
        if os.path.exists(rirfilename):
            rirs = np.load(rirfilename)
            print 'loaded', rirfilename
        else:      
            print rirfilename
            s = mic_center + np.array([dist*np.cos(a*np.pi/180.),dist*np.sin(a*np.pi/180.),dist*np.sin(e*np.pi/180.)])
            
            if not wallfar(mic_center,room):
                raise Exception('microphones too close to the wall')
            if not wallfar(s,room):
                raise Exception('speaker too close to the wall')
                                
            micpos = mic_center + GEO 
            m = np.mean(micpos,0)
            print 'source', np.round(s,2),'dist',np.round(dist,2),  'mic', np.round (m,2),'elevation %.1f' % (180.0/np.pi * math.atan2(s[2]-m[2],math.pow(s[0]-m[0],2) +math.pow(s[1]-m[1],2) ))
                                         
            start = time.clock()                                     
            print 'computing rirs'  , rirfilename
            rirs = RG.rir_generator(SPEEDOFSOUND, FS, micpos, s, ROOM, beta=t60, nsample=rirsamples, mtype='omnidirectional', hp_filter = 1)
            elapsed = time.clock()
            elapsed = elapsed - start
            print rirs.shape, ('%.3fs'%elapsed)
            np.save(rirfilename,rirs)
            
        SIGNAL = np.random.random((FS*2.2,))        
        print 'convolving'
        start = time.clock()                                     
        out=[]
        for i in xrange(8):
            sig = scipy.signal.convolve(SIGNAL,rirs[i,:])
            out.append(sig[:FS*2])
        out = np.vstack(out).T
        elapsed = time.clock()
        elapsed = elapsed - start
            
        print filename, out.shape, ('%.3fs'%elapsed)
        soundfile.write(AUDIOPATH+'/'+filename,out,FS)
    print 
