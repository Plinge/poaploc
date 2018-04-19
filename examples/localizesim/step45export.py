'''
Created on 18.02.2018

@author: pli
'''
import h5py
import glob, os
import numpy as np
from config import DOA_RESOLUTION, WORKPATH, CNNINPUTPATH
from angles import make360
import warnings
from paths import make_sure_path_exists
from feats import running_mean, spikenorm
make_sure_path_exists( CNNINPUTPATH )


        
def exportit(offsets,length,files,average):
    testxx=[]
    testyy=[]
    for i,filepath in enumerate(files):
        print i+1,'/ ',len(files) , filepath
        test_x = np.load(filepath)        
        doaa = os.path.basename(filepath).split('_')[5]
        doa  = int(doaa[1:])
        if average>1:
            test_x = running_mean(test_x, average)
        for offset in offsets:
            xx = test_x[offset:offset+length,:,:,:]
            if xx.shape[0] != length:
                print 'flawed logic'
                continue
            xx,ma  = spikenorm(xx)
            themax.append(ma)
            if xx.max() <= 1e-8:
                print 'max is only', xx.max()
                continue                 
            if xx.max() > 1.0:
                print 'max is', xx.max(),'?'
                continue                    
            testxx.append(xx)    
            test_y = np.zeros((length,360/DOA_RESOLUTION),dtype=np.byte) 
            test_y[:  , int( make360(doa+0.5*DOA_RESOLUTION))/DOA_RESOLUTION] = 1                 
            testyy.append(test_y)
        
        ''' compress from time to time not to flood memory '''
        if ((1+i)&127==0):
            testxx = [np.vstack(testxx),]
            testyy = [np.vstack(testyy),]
            
    testxx = np.vstack(testxx)
    testyy = np.vstack(testyy)
  
    print
    print testxx.shape , '->', testyy.shape    
    print
    
    return testxx, testyy

def getrunfiles(mode,runs):
    res = []
    for r in runs:
        res += glob.glob(WORKPATH+'cor_*_r'+('%02d_'% r)+mode+'.npy'  )
    return res

def exportfull(mode,average):
    global themax  

    themax = []
    
    trainfiles = getrunfiles(mode,range(0,12))
    #trainfiles = getrunfiles(mode,[0,2,4,6])
    
    validfiles = getrunfiles(mode,[12])
    testfiles = getrunfiles(mode,[18])
    
    datalen = 265        
    step = 1
    if average>1:
        datalen = datalen - average
        step = max(1, average/4)
    outfile = CNNINPUTPATH + '/noise_many_'+mode+('_w%02d'%average)+'.hdf5' 
    with h5py.File( outfile, "w") as f:
        
        x,y = exportit(range(0,datalen,2),1,trainfiles,average)
        
        f.create_dataset('X_train', data=x)
        f.create_dataset('Y_train', data=y)
        train_shape = x.shape
        del y,x
        
        x,y = exportit(range(2*step,datalen-2*step,4*step),1,validfiles,average)
        f.create_dataset('X_valid', data=x)
        f.create_dataset('Y_valid', data=y)
        valid_shape = x.shape
        
        x,y = exportit(range(0,datalen,step),1,testfiles,average)
        f.create_dataset('X_test', data=x)
        f.create_dataset('Y_test', data=y)
        test_shape = x.shape
        
        print np.min(themax), np.mean(themax)  ,  np.max(themax), "//",np.median(themax)
        print 'train', train_shape
        print 'valid', valid_shape
        print 'test', test_shape
    print outfile

exportfull('3a_m6_fg',20)

exportfull('3a_m6_dg',20)

#exportfull('3a_m6_dg',0)
#exportfull('3a_m6_fg',0)

