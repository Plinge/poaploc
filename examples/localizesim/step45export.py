'''
Created on 18.02.2018

@author: pli
'''
import h5py
import glob, os
import numpy as np
from config import DOA_RESOLUTION
from angles import make360
import warnings
from paths import make_sure_path_exists

make_sure_path_exists('./cnninput/' )

def makeslice(offset,length,files,shuffle=True):
    testxx=[]
    testyy=[]
    for filepath in files:
        #print os.path.basename(filepath)            
        test_x = np.load(filepath)
        doaa = os.path.basename(filepath).split('_')[5]
        doa  = int(doaa[1:])        
        testxx.append(test_x[offset:offset+length,:,:,:])    
        test_y = np.zeros((length,360/DOA_RESOLUTION),dtype=np.ubyte)
        test_y[:  , int( make360(doa))/DOA_RESOLUTION] = 1                 
        testyy.append(test_y)
        
    print len(testxx),'x', testxx[0].shape
    
    ii = range(len(testxx))
    if shuffle:
        np.random.shuffle(ii)
    testxx = np.vstack((testxx[i] for i in ii))
    testyy = np.vstack((testyy[i] for i in ii))
 
    return testxx,testyy
        
        
def exportit(name,offsets,length,files,shuffle=True):
    testxx=[]
    testyy=[]
    for i,offset in enumerate(offsets):    
        print i+1,'/',len(offsets),
        xx,yy = makeslice(offset, length, files, shuffle)
        testxx.append(xx); testyy.append(yy)
    if len(testxx)<1 : 
        warnings.warn('no data?')
        return
    testxx = np.vstack(testxx)
    testyy = np.vstack(testyy)
    dataname = 'test_' if 'test' in name[:6] else 'train_' 
    hdffile = './cnninput/'+name+'.hdf5'
    with h5py.File(hdffile, "w") as f:
        f.create_dataset(dataname+'x', data=testxx)
        f.create_dataset(dataname+'y', data=testyy)
    print
    print dataname, testxx.shape , '->', testyy.shape
    print hdffile
    print

#exportit('test_quick_3a_m6_fg',[20+32],1,glob.glob('./data/cor_*_t030_*_r00_3a_m6_fg.npy'),False)    
#exportit('train_quick_3a_m6_fg',range(20,168,64),4,glob.glob('./data/cor_*_t030_*_r02_3a_m6_fg.npy'))
exportit('train_more_3a_m6_fg',range(8,168,4),1,glob.glob('./data/cor_*_3a_m6_fg.npy'))
