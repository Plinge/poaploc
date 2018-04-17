'''
Created on 16.04.2018

@author: pli
'''



import h5py
import glob, os
import numpy as np
from evaldoas import load_gt_as_xy
from config import DATAPATH, DOA_RESOLUTION
from angles import make360
from paths import make_sure_path_exists

import matplotlib.pyplot as plt
from matplotlib._cm_listed import cmaps

files = glob.glob('./cnnoutput/*.npy')
for index, filepath in enumerate(files):
    print os.path.basename(filepath)
    
    resmat = np.load(filepath)
    print filepath, resmat.shape
        
    task = os.path.basename(filepath).split('_')[1]    
    doas = load_gt_as_xy( DATAPATH+'groundtruth/'+task.replace('sq','')+"_gt.dat")
    
    plt.figure(index)
    plt.title(os.path.basename(filepath))
    ax =plt.subplot(2,1,1)
    #ax  = plt.axes()
        
    frames  = resmat.shape[0]                 
                     
    duration = frames*6e-3
    ax.imshow(resmat.T, extent=[0,duration,-180,180], origin='lower',   aspect =  frames * 0.00001, cmap='hot' )
    ax.set_xlim(0,duration)
    ax.set_ylim(-180,180)
        
    #ax.set_ylim(-180,180)
    
    ax =plt.subplot(2,1,2)
     
    maxmat = np.zeros_like(resmat)
    for i,j in enumerate(np.argmax(resmat,1)):
        if resmat[i,j]>0.1:
            maxmat[i,j]=1 
     
    ax.imshow(maxmat.T, extent=[0,duration,-180,180], origin='lower',   aspect =  frames * 0.00001, cmap = 'gray_r'  )
    ax.set_xlim(0,duration)
    ax.set_ylim(-180,180)

    
    #ax.scatter(doas[:,0], doas[:,1],s=0.2, c='w', edgecolors = 'none' )
    
    
plt.show()    