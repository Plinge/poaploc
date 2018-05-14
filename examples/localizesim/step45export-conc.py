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

def exportruns(mode,runs,step=1):
    print 'gathering...'    
    xx,yy=[],[]
    runindex=0
    for run,t60 in itertools.product(runs,(15,30,45,60,40,50)):        
        pat = WORKPATH + 'cor_sim_c8_noise_t'+('%03d'%t60)+'_*_r'+('%02d_'% run)+mode+'.npy' 
        #print pat
        files = glob.glob(pat)
        if len(files)<1:
            continue
        print ('%4d'%len(files)),'for',pat        
        np.random.shuffle(files)
        x,y = exportconc(files,step)
        xx.append(x);yy.append(y)
        runindex+=1
        if (runindex+1)&3 == 0:
            xx=[np.vstack(xx),]
            yy=[np.vstack(yy),]
    if len(xx)<1:
        return None,None        
    xx,yy = np.vstack(xx),np.vstack(yy)
    print
    print xx.shape , '->', yy.shape    
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
        l4 = len(trainruns)/3
        for stride in range(4):        
            trainslice = trainruns[stride*l4:(stride+1)*l4]
            if len(trainslice)<1:
                break
            x,y = exportruns(mode,trainslice)
            if x is None:
                break
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
