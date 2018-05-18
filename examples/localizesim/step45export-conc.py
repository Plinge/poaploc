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
import soundfile
import scipy.signal, scipy.fftpack
import itertools

make_sure_path_exists( CNNINPUTPATH )
        
def exportconc(files,step=1):
    testxx=[]
    testyy=[]
    pairx=[];pairy=[]

    for fileindex,filepath in enumerate(files):
        
        test_x =  np.array( np.transpose(np.load(filepath), [0,3,2,1]),dtype=np.float16 )  
        if step>1:
            test_x = test_x[::step,:,:,:]
        doaa = os.path.basename(filepath).split('_')[5]
        doa  = int(doaa[1:])
                
        pairx.append(test_x)      
        pairy.append(int(make360(doa+0.5*DOA_RESOLUTION))/DOA_RESOLUTION)                 
            
        if len(pairy)==2:
            pairx  = np.vstack(pairx)
            datalen = pairx.shape[0]
            shuf = list(range(datalen))
            for k in range(16):
                np.random.shuffle(shuf)
                for m,l in itertools.product(range(28),range(29)):
                    fres = [pairx[i,m,k,l] for i in shuf]
                    pairx[:,m,k,l] = fres
            testxx.append(pairx)
            
            test_y = np.zeros((datalen,360/DOA_RESOLUTION),dtype=np.int8) 
            test_y[:, pairy[0]] = 1
            test_y[:, pairy[1]] = 1
            testyy.append(test_y)
            pairx=[];pairy=[]  
        if (fileindex+1)&15 == 0:
            testxx=[np.vstack(testxx),]
            testyy=[np.vstack(testyy),]
            #print fileindex+1,'/ ',len(files),' .. ',len(testyy[0]) , ' - ',(len(testyy[0])*2)/(fileindex+1), "\r", #filepath
  
    if len(testxx)<1:
        raise Exception('no data?')
    testxx = np.vstack(testxx)
    testyy = np.vstack(testyy)    
    #print
    #print testxx.shape , '->', testyy.shape           
    return testxx, testyy

def exportruns(mode,runs,step=1,t60s=(15,30,45,60,40,50)):
    print 'gathering...'    
    xx,yy=[],[]
    runindex=0
    for run,t60 in itertools.product(runs,t60s):        
        pat = WORKPATH + 'cor_sim_c8_noise_t'+('%03d'%t60)+'_*_r'+('%02d_'% run)+mode+'.npy' 
        #print pat
        files = glob.glob(pat)
        if len(files)<1:
            continue
        print ('%4d'%len(files)),'for',pat        
        np.random.shuffle(files)
        x,y = exportconc(files,step)
        if len(xx)<1:
            xx=x;yy=y
        else:
            xx=np.vstack([xx,x]) ; yy=np.vstack([yy,y])
            del x; del y
    if len(xx)<1:
        return None,None                
    return xx, yy
       
def exportfull(mode):
    global themax  
    themax = []
    trainruns  = list(range(50,60))        
    #trainruns  = list(range(30,35))        
    #trainruns += list(range(40,60))        
    validruns = list(range(35,40))
    testruns = list(range(18,24))
        
    outfile = CNNINPUTPATH + '/noise_circ_conc_'+mode+'.hdf5' 
    with h5py.File( outfile, "w") as f:        
        #x,y = exportruns(mode,trainruns)            
        #f.create_dataset('X_train', data=x, dtype=np.float16)
        #f.create_dataset('Y_train', data=y, dtype=np.int8)
        #train_shape = x.shape
        xset,yset=None,None
        for run,t60set in itertools.product(trainruns,[(15,30,40),(45,60,50)]):        
            trainslice = [run]            
            x,y = exportruns(mode,trainslice,1,t60set)
            if x is None:
                continue
            print x.shape , '->', y.shape
            if xset is None:                
                xset = f.create_dataset('X_train', data=x, dtype=np.float16, maxshape=(None, x.shape[1], x.shape[2],x.shape[3]))                
                yset = f.create_dataset('Y_train', data=y, dtype=np.int8, maxshape=(None, y.shape[1]))
            else:
                xset.resize(xset.shape[0]+len(x),axis=0)
                xset[-len(x):,:,:,:] = x
                yset.resize(yset.shape[0]+len(y),axis=0)
                yset[-len(y):,:] = y
                print 'total=', xset.shape
            #del y,x                
        train_shape = xset.shape
        
        
        x,y = exportruns(mode,validruns,4)
        f.create_dataset('X_valid', data=x, dtype=np.float16)
        f.create_dataset('Y_valid', data=y, dtype=np.int8)
        valid_shape = x.shape
        
        x,y = exportruns(mode,testruns,4)
        f.create_dataset('X_test', data=x, dtype=np.float16)
        f.create_dataset('Y_test', data=y, dtype=np.int8)
        test_shape = x.shape
                
        print 'train', train_shape
        print 'valid', valid_shape
        print 'test ', test_shape
    print outfile

exportfull('3a_m6_fg')
