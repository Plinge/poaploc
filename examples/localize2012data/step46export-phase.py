'''
Created on 18.02.2018

@author: pli
'''
import h5py
import glob, os
import numpy as np
from evaldoas import load_gt_as_nhot
from config import DATAPATH, FILES, DOA_RESOLUTION
from angles import make360
from paths import make_sure_path_exists
from feats import running_mean, spikenorm
import soundfile
import scipy.signal
import scipy.fftpack

make_sure_path_exists('./cnninput/' )

for (task, filepath) in FILES.iteritems():
    print os.path.basename(filepath)
     
    hann = scipy.signal.hanning(512)
    audiodata, fs = soundfile.read(filepath)
    test_x = []
    for i in range(0, audiodata.shape[0]-256*3, 256):
        phases=[]
        for channel in range(8):
            x = audiodata[i:i+512,channel] * hann
            s = scipy.fftpack.fft(x)
            s = np.angle(s[:256],False)
            phases.append(s)
        test_x.append(phases)
        
    test_x = np.transpose( np.array(test_x,dtype=np.float16), [0,2,1])
    xx = test_x.reshape(-1,256,8,1)
        
    yy = load_gt_as_nhot( DATAPATH+'groundtruth/'+task.replace('sq','')+"_gt.dat" , 256./fs , DOA_RESOLUTION)
    yy = np.roll(yy,-18,1)
        
    xlen = xx.shape[0]
    ylen = yy.shape[0]
    print xlen, 'xs', ylen, 'ys'
    if xlen < ylen:        
        yy = yy[:xlen,:] 
    elif ylen < xlen:
        xx = xx[:ylen,:]
    
        
    for average in (0,20):
                
        hdffile = './cnninput/' + task + '_phase.hdf5'
        with h5py.File(hdffile, "w") as f:
            f.create_dataset('X_test', data=xx, dtype=np.float16)
            f.create_dataset('Y_test', data=yy, dtype=np.int8)
            
        print hdffile,
        print xx.shape , '->'  ,yy.shape