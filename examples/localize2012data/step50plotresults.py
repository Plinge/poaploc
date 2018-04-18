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
from matplotlib.ticker import MultipleLocator

files = glob.glob('./cnnoutput/*w20*.npy')
for index, filepath in enumerate(files):
    print os.path.basename(filepath)
    
    resmat = np.load(filepath)
    print filepath, resmat.shape
    resmat = np.roll(resmat,-18,1)
    
    model, epoch,  taskfeat = os.path.basename(filepath).split('.')[0].split('-')    
    task =  taskfeat.split('_')[1]
    
    fig = plt.figure(index)    
    fig.canvas.set_window_title(os.path.basename(filepath))
    ax =plt.subplot(1,2,1)
    #ax  = plt.axes()
        
    frames  = resmat.shape[0]                 
                     
    duration = frames*6e-3
    ax.imshow(resmat.T, extent=[0,duration,-180,180], origin='lower',   aspect =  frames * 0.00001, vmin=0, vmax=1 )
    ax.set_xlim(0,duration)
    ax.set_ylim(-180,180)
    ax.yaxis.set_major_locator(MultipleLocator(90))
    ax.yaxis.set_minor_locator(MultipleLocator(30))    
    ax =plt.subplot(1,2,2)
     
    #maxmat = np.zeros_like(resmat)
    #for i,j in enumerate(np.argmax(resmat,1)):
    #    if resmat[i,j]>0.1:
    #       maxmat[i,j]=1
    maxmat = np.copy(resmat)
    maxmat [maxmat<0.5] = 0 
     
    ax.imshow(maxmat.T, extent=[0,duration,-180,180], origin='lower',   aspect =  frames * 0.00001, vmin=0, vmax=1, cmap = 'gray_r'  )
    ax.set_xlim(0,duration)
    ax.set_ylim(-180,180)
    ax.yaxis.set_major_locator(MultipleLocator(90))
    ax.yaxis.set_minor_locator(MultipleLocator(30))
        
    times, doas = load_gt_as_xy( DATAPATH+'groundtruth/'+task.replace('sq','')+"_gt.dat")  
    ax.scatter(times, doas, s=0.2, c='r', edgecolors = 'none' )
    
    
    testfile = 'cnninput/' + taskfeat + '.hdf5'
    if not os.path.exists(testfile):
        print('missing',testfile)
        continue
    print testfile
    Testing_data = h5py.File(testfile)    
    print Testing_data.keys()
    gtmat = np.array( Testing_data['Y_test'])
    gtmat = np.roll(gtmat,-18,1)

#     ax =plt.subplot(1,3,3)    
#     ax.imshow(gtmat.T, extent=[0,duration,-180,180], origin='lower',   aspect =  frames * 0.00001, cmap = 'gray_r'  )
#     ax.set_xlim(0,duration)
#     ax.set_ylim(-180,180)
#     ax.yaxis.set_major_locator(MultipleLocator(90))
#     ax.yaxis.set_minor_locator(MultipleLocator(30))
    
    
    

plt.show()    