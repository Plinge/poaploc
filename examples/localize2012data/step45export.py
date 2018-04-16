'''
Created on 18.02.2018

@author: pli
'''
import h5py
import glob, os
import numpy as np
from evaldoas import load_gt_as_framed
from config import DATAPATH, DOA_RESOLUTION
from angles import make360
from paths import make_sure_path_exists

make_sure_path_exists('./cnninput/' )

files = glob.glob('./data/cor_fi11sq01_3a_m6_fg.npy')
for index, filepath in enumerate(files):
    print os.path.basename(filepath)
    
    test_x = np.load(filepath)
    task = os.path.basename(filepath).split('_')[1]
    
    doas = load_gt_as_framed( DATAPATH+'groundtruth/'+task.replace('sq','')+"_gt.dat" , 6e-3)
    test_y = np.zeros((doas.shape[0],360/DOA_RESOLUTION),dtype=np.ubyte)
    for frameindex, doav in enumerate(doas):
        doaf = doav[doav>-900]
        for doa in doaf:
            test_y[frameindex, int( make360(doa))/DOA_RESOLUTION] = 1
    
    xlen = test_x.shape[0]
    ylen = test_y.shape[0]
    print xlen, 'xs', ylen, 'ys'
    if xlen < ylen:        
        test_y = test_y[:xlen,:] 
    elif ylen < xlen:
        test_x = test_x[:ylen,:]
    print test_x.shape , '->'  ,test_y.shape
            
    hdffile = './cnninput/' +  os.path.basename(filepath).replace(".npy",".hdf5")
    with h5py.File(hdffile, "w") as f:
        f.create_dataset('test_x', data=test_x)
        f.create_dataset('test_y', data=test_y)
        
    print hdffile