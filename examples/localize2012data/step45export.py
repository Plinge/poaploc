'''
Created on 18.02.2018

@author: pli
'''
import h5py
import glob, os
import numpy as np
from evaldoas import load_gt_as_nhot
from config import DATAPATH, DOA_RESOLUTION
from angles import make360
from paths import make_sure_path_exists
from feats import running_mean, spikenorm

make_sure_path_exists('./cnninput/' )

files = glob.glob('./data/cor_fi*.npy')
for index, filepath in enumerate(files):
    print os.path.basename(filepath)
    task = os.path.basename(filepath).split('_')[1]
    xx, _ = spikenorm( np.load(filepath) )
    xx = np.transpose(xx, [0,3,2,1])
    yy = load_gt_as_nhot( DATAPATH+'groundtruth/'+task.replace('sq','')+"_gt.dat" , 6e-3, DOA_RESOLUTION)
    yy = np.roll(yy,-18,1)
        
    xlen = xx.shape[0]
    ylen = yy.shape[0]
    print xlen, 'xs', ylen, 'ys'
    if xlen < ylen:        
        yy = yy[:xlen,:] 
    elif ylen < xlen:
        xx = xx[:ylen,:]
    
        
    for average in (0,20):
        test_x = running_mean(xx, average)
        test_y = running_mean(yy, average)        
        hdffile = './cnninput/' +  os.path.basename(filepath).replace(".npy",("_w%02d.hdf5"%average))
        with h5py.File(hdffile, "w") as f:
            f.create_dataset('X_test', data=test_x, dtype=np.float16)
            f.create_dataset('Y_test', data=test_y, dtype=np.int8)
            
        print hdffile,
        print test_x.shape , '->'  ,test_y.shape