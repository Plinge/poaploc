'''
Created on 04.02.2018

@author: pli
'''

from config import WORKPATH

import sys
import glob

import matplotlib.pyplot as plt

sys.path.append("../python")
from mslplot import plotmsl

for filepath in glob.glob(WORKPATH+'msl_fi11sq04_*_a0.npy'):
    plotmsl(filepath)
plt.show()