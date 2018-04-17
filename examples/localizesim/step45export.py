'''
Created on 18.02.2018

@author: pli
'''
import h5py
import glob, os
import numpy as np
from config import DOA_RESOLUTION, WORKPATH
from angles import make360
import warnings
from paths import make_sure_path_exists

make_sure_path_exists('./cnninput/' )

global themax  

themax = []

def makeslice(offset,length,files,shuffle=True):
    global thema
    testxx=[]
    testyy=[]
    for filepath in files:
        #print os.path.basename(filepath)                    
        test_x = np.load(filepath)        
        xx = test_x[offset:offset+length,:,:,:]
        xx = np.sqrt(xx*1.0/128.0)
        themax.append( xx.max() )
        xx[xx>1.0]=1.0
        if xx.max() <= 1e-8:
            print 'max is only', xx.max()
            continue                 
        if xx.max() > 1.0:
            print 'max is', xx.max(),'?'
            continue
        doaa = os.path.basename(filepath).split('_')[5]
        doa  = int(doaa[1:])        
        testxx.append(xx)    
        test_y = np.zeros((length,360/DOA_RESOLUTION),dtype=np.ubyte)
        test_y[:  , int( make360(doa))/DOA_RESOLUTION] = 1                 
        testyy.append(test_y)
    if len(testxx)<1: 
        print '--'
        return None,None
    print len(testxx),'x', testxx[0].shape
    
    ii = range(len(testxx))
    if shuffle:
        np.random.shuffle(ii)
    testxx = np.vstack((testxx[i] for i in ii))
    testyy = np.vstack((testyy[i] for i in ii))
     
    return testxx,testyy
        
        
def exportit(offsets,length,files,shuffle=False):
    
    if shuffle:
        print len(files), 'files'
        testxx=[]
        testyy=[]
        for i,offset in enumerate(offsets):    
            print i+1,'/',len(offsets),
            xx,yy = makeslice(offset, length, files)
            if xx is None:
                continue
            testxx.append(xx); testyy.append(yy)
        if len(testxx)<1 : 
            warnings.warn('no data?')
            return
        testxx = np.vstack(testxx)
        testyy = np.vstack(testyy)
    else:
        testxx=[]
        testyy=[]
        for i,filepath in enumerate(files):
            print i+1,'/ ',len(files)
            test_x = np.load(filepath)        
            doaa = os.path.basename(filepath).split('_')[5]
            doa  = int(doaa[1:])
            for offset in offsets:            
                xx = test_x[offset:offset+length,:,:,:]
                xx = np.sqrt(xx*1.0/128.0)
                themax.append( xx.max() )
                xx[xx>1.0]=1.0
                if xx.max() <= 1e-8:
                    print 'max is only', xx.max()
                    continue                 
                if xx.max() > 1.0:
                    print 'max is', xx.max(),'?'
                    continue                    
                testxx.append(xx)    
                test_y = np.zeros((length,360/DOA_RESOLUTION),dtype=np.ubyte)
                test_y[:  , int( make360(doa))/DOA_RESOLUTION] = 1                 
                testyy.append(test_y)
            testxx = [np.vstack(testxx),]
            testyy = [np.vstack(testyy),]
        testxx = np.vstack(testxx)
        testyy = np.vstack(testyy)
  
    print
    print testxx.shape , '->', testyy.shape    
    print
    
    return testxx, testyy

trainfiles = glob.glob(WORKPATH+'cor_*_r00_3a_m6_fg.npy')
for r in [2]:
    trainfiles += glob.glob(WORKPATH+'cor_*_r%02d_3a_m6_fg.npy'  % r)
validfiles = glob.glob(WORKPATH+'cor_*_r04_3a_m6_fg.npy')
testfiles = glob.glob(WORKPATH+'cor_*_r06_3a_m6_fg.npy')
    
with h5py.File( './cnninput/noise_many_3a_m6_fg.hdf5', "w") as f:
    x,y = exportit(range(0,168),1,trainfiles)
    f.create_dataset('X_train', data=x)
    f.create_dataset('Y_train', data=y)
    
    x,y = exportit(range(0,168),1,validfiles)
    f.create_dataset('X_valid', data=x)
    f.create_dataset('Y_valid', data=y)
    
    x,y = exportit(range(0,168),1,testfiles)
    f.create_dataset('X_test', data=x)
    f.create_dataset('Y_test', data=y)

#print np.min(themax), np.mean(themax)  ,  np.max(themax), "//",np.median(themax)
