import numpy as np
import os
import itertools
import matplotlib.pyplot as plt

CMD = 'C:/local_bin/poaptdoa'
MAXTAU = 14 
ARG = '--max-tau '+str(MAXTAU)
CSVFILE = 'circulararry.csv' 
os.system(CMD+' '+ARG+' '+'circulararray.wav >'+CSVFILE)

with open(CSVFILE,'r') as f:
    lines = f.readlines()
HEADERS = lines[0].split()
print 'read file with headers', HEADERS

lines= lines[1:]
vals = lines[0].split()
ENTRYCOUNT = len(lines)
ENTRYLEN = len(vals)
TDOAS = ENTRYLEN - 4

print 'parsing..'
''' parse data to numbers '''
data = np.zeros((ENTRYCOUNT,ENTRYLEN))
for index,line in enumerate(lines):
    data[index,:] = [float(v) for v in line.split()]
print 'total', data.shape
''' fist time slot '''
first = data[ data[:,0]==0 ]
BANDS = range(int(np.max(first[:,1])+1))
MICS  = range(int(np.max(first[:,3])+1))
PAIRS = first[first[:,1]==0][:,2:4].astype(np.int)

print 'computing sums..'

values={}
for pair,band in itertools.product(PAIRS,BANDS):
    values[ pair[0] + pair[1] *10 + band*100 ]=np.zeros((TDOAS))

for entry in data:
    pair = entry[2:4].astype(np.int)
    band = int(entry[1])
    values[ pair[0] + pair[1] *10 + band*100 ] += entry[4:]

print 'plotting..'

for pair in PAIRS:
    plt.subplot(len(MICS)-1,len(MICS)-1,pair[1] + (len(MICS)-1)*pair[0])
    plt.title( "mics %d,%d"%(pair[0],pair[1]))
    for band in BANDS:
        plt.plot(range(-MAXTAU,MAXTAU+1),values[ pair[0] + pair[1] *10 + band*100 ])

plt.show()
