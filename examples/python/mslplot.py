'''
Created on 17 Jan 2016

@author: plingea
'''
import numpy as np
from statsmodels.discrete.tests.test_sandwich_cov import filepath

def msl2mat(nn, framerate = 8, framecnt = 1024, samplingrate = 48000.0, angleprecision=1.0):
    subframerate = float(samplingrate)/float(framecnt)
    datarate     = float(subframerate) / np.ceil( float(subframerate)/float(framerate) ) 
    dataoffset   = 1.0 / float(subframerate)

    frames = int(np.ceil(np.max(nn[:,1])*datarate-dataoffset))
    start =  int(np.floor(np.min(nn[:,1])*datarate-dataoffset))
    timesofframes = {}
    for frame in xrange(start,frames+1):
        timesofframes[frame]=set()
    msl = np.zeros((int(360/angleprecision+1),frames+1-start))
    for row in nn:
        timef = row[1]
        frame = int(datarate * timef - dataoffset)        
        timesofframes[frame] = timesofframes[frame] | set([timef]) 
        angle = int((180 + row[2])/angleprecision)
        value = row[4]
        msl[angle,frame-start] += value
    
    counts = np.zeros((frames+1-start))
    for frame in xrange(start,frames+1):
        counts[frame-start] = len(timesofframes[frame])
    counts[counts==0]=1
    msl = msl / counts
        
    return msl

def msl2mat_tf(nn, framerate = 8, framecnt = 1024, samplingrate = 48000.0, bandcnt=16):
    subframerate = float(samplingrate)/float(framecnt)
    datarate     = float(subframerate) / np.ceil( float(subframerate)/float(framerate) ) 
    dataoffset   = 1.0 / float(subframerate)

    frames = int(np.ceil(np.max(nn[:,1])*datarate-dataoffset))
    start =  int(np.floor(np.min(nn[:,1])*datarate-dataoffset))
    timesofframes = {}
    for frame in xrange(start,frames+1):
        timesofframes[frame]=set()
    msl = np.zeros((frames+1-start,bandcnt))
    for row in nn:
        timef = row[1]
        frame = int(datarate * timef - dataoffset)        
        timesofframes[frame] = timesofframes[frame] | set([timef])         
        values = row[5:6+bandcnt]
        msl[frame-start,:] += values
    
    counts = np.zeros((frames+1-start))
    for frame in xrange(start,frames+1):
        counts[frame-start] = len(timesofframes[frame])
    counts[counts==0]=1
    counts=counts.reshape(-1,1) 
    msl = msl / np.repeat(counts,bandcnt,1)
        
    return msl

def msl2mat_af(nn, framerate = 8, framecnt = 1024, samplingrate = 48000.0, angleprecision=1.0, bandcnt=16):
    
    msl = np.zeros((int(360/angleprecision+1),bandcnt))
    for row in nn:
        timef = row[1]
        #frame = int(datarate * timef - dataoffset)        
        #timesofframes[frame] = timesofframes[frame] | set([timef]) 
        angle = int((180 + row[2])/angleprecision)
        values = row[5:6+bandcnt]
        msl[angle,:] += values
            
    return msl


def msl2mat_fix(nn, framerate = 8, framecnt = 1024, samplingrate = 48000.0, angleprecision=1.0):
    
    datarate     = 1.0 /float(framerate)  
    

    frames = int(np.ceil(np.max(nn[:,1])*datarate))
    timesofframes = {}
    for frame in xrange(frames+1):
        timesofframes[frame]=set()
    msl = np.zeros((int(360/angleprecision+1),frames+1))
    for row in nn:
        timef = row[1]
        frame = int(datarate * timef)        
        timesofframes[frame] = timesofframes[frame] | set([timef]) 
        angle = int((180 + row[2])/angleprecision)
        value = row[4]
        msl[angle,frame] += value
    
    counts = np.zeros((frames+1))
    for frame in xrange(frames+1):
        counts[frame] = len(timesofframes[frame])
    counts[counts==0]=1
    msl = msl / counts
        
    return msl

def dat2mat_fix(nn, framerate = 8, angleprecision=1.0, tcol=1,acol=2,vcol=4):
    
    datarate     = float(framerate)  
    frames = int(np.ceil(np.max(nn[:,tcol])*framerate))
    timesofframes = {}
    for frame in xrange(frames+1):
        timesofframes[frame]=set()
    msl = np.zeros((int(360/angleprecision+1),frames+1))
    for row in nn:
        timef = row[tcol]
        frame = int(datarate * timef)        
        timesofframes[frame] = timesofframes[frame] | set([timef]) 
        angle = int((180 + row[acol])/angleprecision)
        value = row[vcol]
        msl[angle,frame] += float(value)
    
    counts = np.zeros((frames+1))
    for frame in xrange(frames+1):
        counts[frame] = len(timesofframes[frame])
    counts[counts==0]=1
    msl = msl / counts
        
    return msl

''' '''
import sys,os
import matplotlib.pyplot as plt
from matplotlib.ticker import MultipleLocator

def plotmsl(filepath):
    adata = np.load(filepath)
    if len(adata)<1:
        print >> sys.stderr, 'no data in', filepath
        return
    
    fig = plt.figure()
    ax  = plt.axes()
    framerate = 8.0    
    frames  = 2 +  np.max(adata[:,1])*framerate                 
    msl = msl2mat(adata,framerate)                 
    duration = frames/framerate
    ax.imshow(msl, extent=[0,duration,-180,180], origin='lower', 
              aspect =  frames * 0.0002 )    
    #times = np.arange(frames).astype(np.double) / framerate
    ax.set_ylim(-180,180)
    ax.set_xlim(0,duration)
    ax.yaxis.set_major_locator(MultipleLocator(90))
    ax.yaxis.set_minor_locator(MultipleLocator(30))
    plt.title(os.path.basename(filepath))