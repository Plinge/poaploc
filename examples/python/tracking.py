# -*- coding: utf-8 -*-
'''
Created on Dec 16, 2015

@author: aplinge

This is an implementation of the following paper:

[2] Plinge, A., & Fink, G. A. (2013). 
    Online Multi-Speaker Tracking Using Multiple Microphone Arrays Informed by Auditory Scene Analysis.
    European Signal Processing Conference (EUSIPCO); Marrakesh, Morocco.

'''
# libs
import itertools
import os
import platform
import pandas as pd
import numpy as np
import glob
import sys
from paths import make_sure_path_exists
import angles

CMDALL = '--precision 1 --elevation-precision 5 --min-likelihood 0.01 --skip 3 \
--post-min-bands 4 --post-min-energy=-40 \
--sampling-frequency 48000 \
--maxelevation --max-elevation 45 \
--frame-length 12 --frame-step 6 '
CMD =  "--gain-mode 3 --gain 40 --gain-smooth 0.05 --gain-max 24"
#'poa5sg' : "--gain-mode 0 --gain 30 --post-time 0.5 --maxelevation --spike-ath 0",

ARG =  "--bands 16 --fmin 300 --fmax 3000 --gamma 0.5 --spike-mth 6 --spike-ath 0.0"

ARGS_POST = '--post-time 0.5 --post-min-bands 4 --post-min-energy=-40'

if platform.system() == 'Linux':
    DATAPATH = '/vol/big/aplinge/recordings/'
    BINPATH = ""
    OUTPATH = '/data/aplinge/tracking/'
else:
    DATAPATH = 'E:/data/recordings/'
    OUTPATH = 'E:/data/experiments/'
    BINPATH = "C:\\local_bin\\"

def readcsv(filename):
    r = pd.read_csv(filename, delimiter=r"\s+")
    return np.array(r)[:,:]

# time    micarray    angle    elevation    energy    value
# s0    s1    s2    s3    s4    s5    s6    s7    s8    s9    s10    s11    s12    s13    s14    s15    
# gain    meanen

FRAMERATE = 8
BANDS = 16

MODE_PRES = { 'msl' : ARGS_POST ,
              'msn' : ARGS_POST + ' --poap --argmax --sum-bands' ,
              'msr' : '--post-time 0'}
    
def compute_msx(files,args,destdir,mode='msl',redo=False):
    destfull = destdir+'/'
    make_sure_path_exists(destfull)

    for (tablepre,filepath) in files.iteritems():
        table = tablepre     
        table = mode +  '_' + table + '_poa5eg_3a_g05_m6'  
        csvpath = destfull+table+'.csv'        
        if os.path.exists(csvpath) and not redo:
            continue
         
        compute_poapmloc(filepath,
                         '--quiet ' + CMDALL + ' ' +  MODE_PRES[mode] + ' ' + ARG + ' ' + args + ' ' + CMD ,
                         table,
                         csvpath,
                         destfull,redo=redo)
         
def compute_poapmloc(filepath,args,table,csvpath,npypath,bands=16,redo=False):
    print table
    cmd2 = BINPATH + 'poapmloc  ' +args  
    cmd2 += ' ' + filepath
    cmd2 += ' >' + csvpath
    
    if not os.path.exists(csvpath) or redo:
        print cmd2        
        if os.system(cmd2) != 0:
            raise RuntimeError()
    
    export_poapmloc(table,csvpath,npypath,bands)

def _get_npfile(table,csvpath,npypath,arrayIndex):
    npfile = npypath+table
    if npfile[-3:] != '_a'+str(arrayIndex):
        npfile = npfile +'_a'+str(arrayIndex)
    return npfile

def export_poapmloc(table,csvpath,npypath,bands=16):             
    #print FRAMERATE, BANDS
    data = readcsv(csvpath)
    for arrayIndex in range(9):
        adata = data[data[:,1]==arrayIndex]
        if len(adata)<1:
            continue
        times  = adata[:,[0,2,3]]
        frames = (FRAMERATE * times[:,0]).astype(int).reshape(-1,1)
        sdata  = adata[:,5:6+bands]        
        nn = np.hstack((frames,times,sdata))
        npfile =_get_npfile(table, csvpath, npypath, arrayIndex)
        np.save(npfile+'.npy',nn)
        print csvpath,'=>',npfile

import emtrack
from npfiles import anglesFromMsl, clusteredAngleToRow

def compute_em(destdir,
               minval=0.05, bands = 7, support = 100,
               num_arrays=3,
               redo=True,pattern='',win=None):
        
    SPLIT   = 22.0
    JOINT   = 12.0
    destfull = destdir+'/'
    compute_em_sub(destfull, 
               minval, bands, support,
               SPLIT,JOINT,
               num_arrays,
               redo,pattern,win)
    
def compute_em_sub(destfull, 
               minval=0.05, bands = 7, support = 100,
               splitt=22,joint=12,
               num_arrays=3,
               redo=True,pattern='',win=None):  
    
    VERBOSE =   2
    EMTIMESTEP = 1.0/FRAMERATE    
    EMWINDOW = 2.0
    
    if win is None:
        win = EMWINDOW
        step = EMTIMESTEP
    
    suf = ['w%03d' % (win*100)]
    step = win*0.25
    if splitt!=22  or joint!=12:
        suf.append('s%dj%d'%(splitt,joint))
    if bands!=7:
        suf.append('b%d'%bands)
    if minval!=0.05:
        suf.append('h%d' % (minval*100))
    if len(suf)<1: 
        suf=None
    else:
        suf = '_'.join(suf)
    
    filepat='msl*'+pattern+'*_a0.npy'
    filez=sorted(glob.glob(destfull+filepat))
    if len(filez)<1:
        print >>sys.stderr, 'NO FILES IN', destfull,'MATCHING',filepat
    
    for filepath0 in filez:
        alldata = np.load(filepath0)
        bandcount = alldata.shape[1] - 5
                   
        outpath = filepath0.replace('msl','aem')
        if suf is not None:
            outpath = outpath.replace('.npy','_'+suf+'.npy')             
        if os.path.exists(outpath) and not redo:
            print outpath, 'exists, skipped.'
            continue
        
        print outpath
        
        for micarray in range(num_arrays):
            filepath = filepath0.replace('_a0.npy','_a'+str(micarray)+'.npy')
            if micarray>0 and filepath == filepath0:
                raise RuntimeError()
            try:
                alldata = np.load(filepath)
            except:
                continue
            outpath = filepath.replace('msl','aem')
            if suf is not None:
                outpath = outpath.replace('.npy','_'+suf+'.npy')                
            ts, te = min(alldata[:,1]), max(alldata[:,1])
            ts = np.floor(ts)
            te = np.ceil(te)
            
            print filepath,ts,te        
            em = emtrack.SpecAngleEM()
            em.setverbousity(0)
            results=[]
            for time in np.arange(ts,te+EMTIMESTEP,step):                        
                res = anglesFromMsl(alldata, time, win*0.5, minval, bandcount)
                                     
                n = 0            
                if len(res)>0:
                    em.fit(res,splitthreshold=splitt,jointhreshold=joint)                                    
                    em.join_close(jointhreshold=joint, mincor=0.0)                
                    a = em.getcentroids()                    
                    for x in a:
                        if x.getbands()>bands and x.getv()<24.0 and x.support>support:
                            if n==0:
                                print "%6.2f %s" % (time, str(x))
                            else:
                                print "      "+' '+str(x)
                            n=n+1                                            
                            results.append(clusteredAngleToRow(int(time*FRAMERATE),time,x))                            
                            
                        elif VERBOSE>2:
                            print " "*10, "skipped", 
                            print "%.1f"%x.angle,
                            print "+-%.1f"%x.getv(),
                            print "sup",x.support,
                            print "bands",x.getbands()
                              
                if n==0:
                    print "%6.2f ---" % (time)
                
            results=np.array(results)
            np.save(outpath, results)
             