'''
Created on 24.02.2018

@author: pli
'''

import sys,os
from numpy import fabs
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
ROOM3        = np.array([4.6,3.8,2.7])     # Room dimensions [x y z] (m)
MIC_CENTER   = np.array([2.,1.5,1.5]) 
MIC_POS      = MIC_CENTER + GEO # Receiver position [x y z] (m)

make_sure_path_exists(RIRPATH)
make_sure_path_exists(AUDIOPATH)

for r,t60,a in itertools.product(range(12,14),[0.3,0.6,0.9],range(0,360,5)):
    
    filename = 'sim_c8_noise_t%03d_a%03d_r%02d.wav' % (int(t60*100),int(a),r)
    if (os.path.exists(AUDIOPATH+'/'+filename)):
        continue    
    room = ROOM3 
    rirfilename = RIRPATH + 'rir_c8_t%03d_a%03d_r%02d.npy' % (int(t60*100),int(a),r)     
        
    if os.path.exists(rirfilename):
        rirs = np.load(rirfilename)
        print 'loaded', rirfilename
    else:
        tries=0  
        while tries < 100:
            tries+=1
            dist = 0.7 + np.random.random_sample()
            offset = np.array( [ np.random.random_sample() * 2.0 - 1.0, np.random.random_sample() * 2.0 - 1.0,  np.random.random_sample() * 0.5 - 0.25 ] )
            m = MIC_CENTER + offset
            s = m  + np.array([dist*np.cos(a*np.pi/180.),dist*np.sin(a*np.pi/180.),np.random.random_sample() * 0.5])
            micpos = MIC_POS + offset
            
            for dim in range(3):            
                if s[dim]<0.5:
                    continue
                if room[dim]-s[dim]<0.5:
                    continue     
                if m[dim]<0.5:
                    continue
                if room[dim]-m[dim]<0.5:
                    continue     
            
            print 'source', np.round(s,2),'dist',np.round(dist,2),  'mic', np.round (np.mean(MIC_POS,0) + offset,2),'try',tries
            break 
        if tries >= 100 :
            continue
        print 'computing rirs'  , rirfilename
        rirs = RG.rir_generator(SPEEDOFSOUND, FS, micpos, s, room, beta=t60, nsample=NUMSAMPLES, mtype='omnidirectional', hp_filter = 1)
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
