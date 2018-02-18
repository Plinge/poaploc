'''
Created on 18.02.2018

@author: pli
'''
import numpy as np
import itertools
import glob,os
import matplotlib.pyplot as plt


files = glob.glob('./data/cor_fi11sq01_3a_m6_*.npy')
for index, file in enumerate(files):
    data = np.load(file)
    print file, data.shape
    
    print 'computing sums..'
    BANDS=range(data.shape[2])
    PAIRS=range(data.shape[3])
    TDOAS=data.shape[1]
    MICS=8
    MAXTAU=14
    
    values={}
    for pair,band in itertools.product(PAIRS,BANDS):
        values[ pair + band*100 ]=np.mean( data[100:160,:,band,pair],axis=0)
    
    print 'plotting..'
    PAIRIDX={}
    for n,(i,j) in enumerate(itertools.combinations(range(MICS),2)):
        PAIRIDX[n] = (i,j)
    
    fig = plt.figure(index)     
    fig.canvas.set_window_title(os.path.basename(file))  
    for pair in PAIRS:
        pa=PAIRIDX[pair]
        plt.subplot((MICS)-1,(MICS)-1,pa[1] + ((MICS)-1)*pa[0])
        plt.title( "mics %d,%d"%(pa[0],pa[1]))
        for band in BANDS:
            plt.plot(range(-MAXTAU,MAXTAU+1),values[ pair + band*100 ])
    
    #plt.tight_layout()

print 'ok'
plt.show()
