'''
Created on 24.02.2018

@author: pli
'''

import sys,os,glob
from numpy import fabs
import math, time
sys.path.append('../python')
import rirgenerator as RG
import numpy as np
import soundfile
from config import GEO, AUDIOPATH, RIRPATH, SPEECHPATH
import scipy.signal
from paths import make_sure_path_exists
import itertools

SPEEDOFSOUND = 343   # Sound velocity (m/s)
FS = 48000           # Sample frequency (samples/s)
NUMSAMPLES = 1<<15   # Number of samples            
   
make_sure_path_exists(RIRPATH)
make_sure_path_exists(AUDIOPATH)

for run,t60 in itertools.product(range(0,20),[0.15,0.3,0.45,0.6,0.75,0.9,1.05,1.2]):
    r = run + 30  
    speaker = int(run/4) + 1
    speechfiles = glob.glob(SPEECHPATH+'/speech'+str(speaker)+'*.wav')
    if len(speechfiles)<1:
        print 'no speech files for', speaker
        continue
    OUTSAMPLES = int(1.2 * FS)
    rirsamples = min( NUMSAMPLES, t60*FS)
    angles = range(0,360,5)
    np.random.shuffle(angles)
    for a in angles:
        filename = 'sim_c8_speech_t%03d_a%03d_r%02d.wav' % (int(t60*100),int(a),r)
        if (os.path.exists(AUDIOPATH+'/'+filename)):
            print filename, 'found'
            continue    
        rirfilename = RIRPATH + 'rir_c8_t%03d_a%03d_r%02d.npy' % (int(t60*100),int(a),r)             
        if not os.path.exists(rirfilename):            
            continue
        print 'loading', rirfilename    
        if not os.path.exists(rirfilename):
            raise RuntimeError('missing rir')
                    
        rirs = np.load(rirfilename)
        np.random.shuffle(speechfiles)            
        print 'loading', speechfiles[0]
        signal, fs = soundfile.read(speechfiles[0])
        if fs != FS:
            raise 'sample rate mismatch'
        l = signal.shape[0]
        o = np.random.randint(0, l - OUTSAMPLES-1)
        signal = signal[o:o+OUTSAMPLES]        
        print 'convolving'
        start = time.clock()                                     
        out=[]
        for i in xrange(8):
            sig = scipy.signal.convolve(signal,rirs[i,:])
            out.append(sig[:FS*2])
        out = np.vstack(out).T
        elapsed = time.clock()
        elapsed = elapsed - start
            
        print filename, out.shape, ('%.3fs'%elapsed)
        soundfile.write(AUDIOPATH+'/'+filename,out,FS)
    print 
