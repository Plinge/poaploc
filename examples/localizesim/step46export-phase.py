'''
Created on 18.02.2018

@author: pli
'''
import h5py
import glob, os
import numpy as np
from config import DOA_RESOLUTION, AUDIOPATH, CNNINPUTPATH
from angles import make360
import warnings
from paths import make_sure_path_exists
from feats import running_mean, spikenorm
import soundfile
import scipy.signal, scipy.fftpack
make_sure_path_exists( CNNINPUTPATH )


        
def exportit(files,shuffle=False):
    testxx=[]
    testyy=[]
    pairx=[];pairy=[]
    hann = scipy.signal.hanning(512)
    for i,filepath in enumerate(files):
        audiodata, fs = soundfile.read(filepath)
        test_x = []
        for i in range(256*4, audiodata.shape[0]-256*4, 256):
            phases=[]
            for channel in range(8):
                x = audiodata[i:i+512,channel] * hann
                s = scipy.fftpack.fft(x)
                s = np.angle(s[:256],False)
                phases.append(s)
            test_x.append(phases)
            
        test_x = np.transpose( np.array(test_x,dtype=np.float16), [0,2,1])
        test_x = test_x.reshape(-1,256,8,1)
        #test_x = np.load(filepath)        
        doaa = os.path.basename(filepath).split('_')[4]
        doa  = int(doaa[1:])
        datalen = test_x.shape[0]
            
        #test_y = np.zeros((datalen,360/DOA_RESOLUTION),dtype=np.int8) 
        #test_y[:  , int( make360(doa+0.5*DOA_RESOLUTION))/DOA_RESOLUTION] = 1
        
        pairx.append(test_x)      
        pairy.append(int( make360(doa+0.5*DOA_RESOLUTION))/DOA_RESOLUTION)                 
            
        if len(pairy)==2:
            pairx  = np.vstack(pairx)
            datalen = pairx.shape[0]
            shuf = list(range(datalen))
            
            for k in range(256):
                np.random.shuffle(shuf)
                for m in range(8):
                    fres = [pairx[i,k,m,0] for i in shuf]
                    pairx[:,k,m,0] = fres
            
            testxx.append(pairx)
            test_y = np.zeros((datalen,360/DOA_RESOLUTION),dtype=np.int8) 
            test_y[:, pairy[0]] = 1
            test_y[:, pairy[1]] = 1
            testyy.append(test_y)
            
        if (i+1)&63 == 0:
            testxx=[np.vstack(testxx),]
            testyy=[np.vstack(testyy),]
            print i+1,'/ ',len(files),' .. ',len(testyy[0]) , "\r", #filepath
  
  
    if len(testxx)<1:
        raise Exception('no data?')
        
    if shuffle:      
        ii = range(len(testxx)) 
        np.random.shuffle(ii)               
        testxx = np.vstack([testxx[i] for i in ii])
        testyy = np.vstack([testyy[i] for i in ii])
    else:
        testxx = np.vstack(testxx)
        testyy = np.vstack(testyy)
    
    print
    print testxx.shape , '->', testyy.shape    
       
    return testxx, testyy

def getrunfiles(runs,shuffle=False):
    res = []
    for r in runs:
        pat = AUDIOPATH + 'sim_c8_noise*_r'+('%02d'% r)+'.wav' 
        res += glob.glob(pat)
    if shuffle:
        np.random.shuffle(res)
    if len(res)<1:
        raise Exception('No files for '+pat)
    return res

def exportfull():
    global themax  

    themax = []
        
    trainfiles = getrunfiles(range(30,35),True)        
    trainfiles+= getrunfiles(range(40,50),True)        
    validfiles = getrunfiles(range(35,40),True)
    testfiles = getrunfiles(range(18,24),True)
        
    step = 1
    
    outfile = CNNINPUTPATH + '/noise_circ_phase'+'.hdf5' 
    with h5py.File( outfile, "w") as f:        
 
        x,y = exportit(trainfiles)
        f.create_dataset('X_train', data=x, dtype=np.float16)
        f.create_dataset('Y_train', data=y, dtype=np.int8)
        train_shape = x.shape
        
        x,y = exportit(validfiles)
        f.create_dataset('X_valid', data=x, dtype=np.float16)
        f.create_dataset('Y_valid', data=y, dtype=np.int8)
        valid_shape = x.shape
        
        x,y = exportit(testfiles)
        f.create_dataset('X_test', data=x, dtype=np.float16)
        f.create_dataset('Y_test', data=y, dtype=np.int8)
        test_shape = x.shape
                
        print 'train', train_shape
        print 'valid', valid_shape
        print 'test ', test_shape
    print outfile

exportfull()


