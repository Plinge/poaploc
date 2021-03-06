'''
Created on 18.02.2018

@author: pli
'''
import h5py, tables
import glob, os, sys
sys.path.append('../python')
import numpy as np
from config import DOA_RESOLUTION, AUDIOPATH, CNNINPUTPATH
from angles import make360
import warnings
from paths import make_sure_path_exists
from feats import running_mean, spikenorm
import soundfile
import scipy.signal, scipy.fftpack
import itertools

make_sure_path_exists( CNNINPUTPATH )
        
def exportconc(files):
    testxx=[]
    testyy=[]
    pairx=[];pairy=[]
    hann = scipy.signal.hanning(512)
    for fileindex,filepath in enumerate(files):
        #print filepath
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
        
        test_x = np.transpose(np.array(test_x,dtype=np.float16), [0,2,1])
        test_x = test_x.reshape(-1,256,8,1)
        #test_x = np.load(filepath)        
        doaa = os.path.basename(filepath).split('_')[4]
        doa  = int(doaa[1:])
      
        pairx.append(test_x)      
        pairy.append(int(make360(doa+0.5*DOA_RESOLUTION)/DOA_RESOLUTION)) 
            
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
            
            test_y = np.zeros((datalen,int(360/DOA_RESOLUTION)),dtype=np.int8) 
            test_y[:, pairy[0]] = 1
            test_y[:, pairy[1]] = 1
            testyy.append(test_y)
            pairx=[];pairy=[]  
        if (fileindex+1)&63 == 0:
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

def exportruns(runs,stype='noise'):
    print('gathering...')
    xx,yy=[],[]
    for run,t60 in itertools.product(runs,(15,30,45,60,40,50)):        
        pat = AUDIOPATH + 'sim_c8_'+stype+'*_t'+('%03d'%t60)+'_*_r'+('%02d'% run)+'.wav' 
        files = glob.glob(pat)
        if len(files)<1:
            continue
        print(('%4d'%len(files)),'for',pat)
        np.random.shuffle(files)
        x,y = exportconc(files)
        xx.append(x);yy.append(y)
        
    xx,yy = np.vstack(xx),np.vstack(yy)
    print (xx.shape , '->', yy.shape)
    return xx, yy
       
def exportfull():
    global themax  
    themax = []
    trainruns  = list(range(40,60))        
    validruns = list(range(35,40))
    testruns = list(range(30,35))
        
    outfile = CNNINPUTPATH + '/noise_circ_conc_phase'+'.hdf5' 
    #with h5py.File( outfile, "w") as f:
    with tables.open_file(outfile, mode="w") as f:
        xa = f.create_earray(f.root,'X_train', tables.Atom.from_dtype(np.dtype('float16')),(0, 256, 8, 1))
        ya = f.create_earray(f.root,'Y_train', tables.Atom.from_dtype(np.dtype('int8')),(0,int(360/DOA_RESOLUTION)))       
        for run in trainruns:            
            x,y = exportruns([run,])
            xa.append(x)
            ya.append(y)
            print (xa.shape,'->',ya.shape)        
        train_shape = xa.shape
        
        x,y = exportruns(validruns)
        f.create_array(f.root,'X_valid', x.astype(np.float16))
        f.create_array(f.root,'Y_valid', y.astype(np.int8))
        valid_shape = x.shape
        
        
        x,y = exportruns(testruns,'speech')
        f.create_array(f.root,'X_test', x.astype(np.float16))
        f.create_array(f.root,'Y_test', y.astype(np.int8))
        test_shape = x.shape
                
        #print('train', train_shape)
        print('valid', valid_shape)
        print('test ', test_shape)
        
    print(outfile)

print("DOA quantized to ", DOA_RESOLUTION)
  
exportfull()
