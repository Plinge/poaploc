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
import itertools
from paths import make_sure_path_exists
from feats import running_mean, spikenorm
make_sure_path_exists( CNNINPUTPATH )
       
def exportit(step,length,files,average,shuffle=False):
    testxx=[]
    testyy=[]
    for i,filepath in enumerate(files):
        test_x = np.load(filepath)        
        doaa = os.path.basename(filepath).split('_')[5]
        doa  = int(doaa[1:])
        datalen = test_x.shape[0]
        if average>1:
            test_x = running_mean(test_x, average)        
        for offset in range(0,datalen-average-2,step):
            xx = test_x[offset:offset+length,:,:,:]
            if xx.shape[0] != length:
                print 'flawed logic'
                continue
            xx,ma  = spikenorm(xx)
            themax.append(ma)
            if xx.max() <= 1e-3:
                print 'max is only', xx.max(), filepath, offset
                continue                 
            if xx.max() > 1.0:
                print 'max is', xx.max(),'?'
                continue                    
            testxx.append(np.array( np.transpose(xx, [0,3,2,1]),dtype=np.float16 ) )    
            test_y = np.zeros((length,360/DOA_RESOLUTION),dtype=np.int8) 
            test_y[:  , int( make360(doa+0.5*DOA_RESOLUTION))/DOA_RESOLUTION] = 1                 
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

def getrunfiles(mode,runs,t60s=[15,30,45],shuffle=False,type='noise'):
    res = []
    for r, t in itertools.product(runs,t60s):
        pat = WORKPATH+'cor_sim_c8_'+type+'_t'+('%03d' % t)+'*_r'+('%02d_'% r)+mode+'.npy' 
        res += glob.glob(pat)
    if shuffle:
        np.random.shuffle(res)
    if len(res)<1:
        raise Exception('No files for '+pat)
    return res

def exportfull(mode,average,small=0,t60s=[15,30,45,60]):
    global themax  

    themax = []
        
    if small==1:
        trainfiles = getrunfiles(mode,range(30,35),t60s)                      
        desc = 'some'
    elif small==2:
        trainfiles = getrunfiles(mode,[30,40,45],t60s)                      
        desc = 'few'
    else:      
        trainfiles = getrunfiles(mode,range(30,35),t60s)  
        trainfiles+= getrunfiles(mode,range(40,60),t60s)
        trainfiles+= getrunfiles(mode,range(18,24),t60s)                
        desc = 'all'
    if small==2:
        validfiles = getrunfiles(mode,[37],t60s)
    else:
        validfiles = getrunfiles(mode,range(35,40),t60s)
    if small==2:
        testfiles = getrunfiles(mode,[35],t60s,False,'speech')
    else:
        testfiles = getrunfiles(mode,range(30,50),t60s,False,'speech')
    step = 1
    if average>1:        
        step = max(1, average/4)
    outfile = CNNINPUTPATH + '/noise_'+desc+'_circ_'+mode+('_w%02d'%average)+'.hdf5' 
    with h5py.File( outfile, "w") as f:        
        l4 = len(trainfiles)/16
        for stride in range(17):        
            trainslice = trainfiles[stride*l4:(stride+1)*l4]
            if len(trainslice)<1:
                break
            x,y = exportit(1,1,trainslice,average)
            if stride == 0:                
                xset = f.create_dataset('X_train', data=x, dtype=np.float16, maxshape=(None, x.shape[1], x.shape[2],x.shape[3]))                
                yset = f.create_dataset('Y_train', data=y, dtype=np.int8, maxshape=(None, y.shape[1]))
            else:
                xset.resize(xset.shape[0]+len(x),axis=0)
                xset[-len(x):,:,:,:] = x
                yset.resize(yset.shape[0]+len(y),axis=0)
                yset[-len(y):,:] = y
                print 'stride', stride+1, 'total=', xset.shape
        del y,x                
        train_shape = xset.shape
#         x,y = exportit(1,1,trainfiles,average)
#         f.create_dataset('X_train', data=x, dtype=np.float16)
#         f.create_dataset('Y_train', data=y, dtype=np.int8)
#         train_shape = x.shape
        
        x,y = exportit(step,1,validfiles,average)
        f.create_dataset('X_valid', data=x, dtype=np.float16)
        f.create_dataset('Y_valid', data=y, dtype=np.int8)
        valid_shape = x.shape
        
        x,y = exportit(step,1,testfiles,average)
        f.create_dataset('X_test', data=x, dtype=np.float16)
        f.create_dataset('Y_test', data=y, dtype=np.int8)
        test_shape = x.shape
        
        print np.min(themax), np.mean(themax)  ,  np.max(themax), "//",np.median(themax)
        print 'train', train_shape
        print 'valid', valid_shape
        print 'test ', test_shape
    print outfile

#exportfull('3a_m6_fg',0,2)
exportfull('3a_m6_cg',0,2)
exportfull('3a_m6_cg',0,1)
#exportfull('3a_m6_fg',20)
#exportfull('3a_m6_dg',20)
#exportfull('3a_m6_dg',0)


