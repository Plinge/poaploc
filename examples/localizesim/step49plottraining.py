'''
Created on 17.04.2018

@author: pli
'''

import json
import matplotlib.pyplot as plt
import numpy as np

##import pickle
##f = open('cnn_axel_history.pickle','rb')
##history = pickle.load(f)
##f.close()

##f = open('cnn_axel_history.json','w')
##f.write(json.dumps(history))
##f.close()
f = open('cnn_axel_history.json','r')
history = json.loads(f.read())
#print(history)

train=[]
valid=[]
doa=[]

for entry in history:
    train.append(entry['acc'])
    valid.append(entry['val_acc'])
    doa.append(entry['val_doa'] )
    
    
fig = plt.figure()
ax = plt.gca()
xs = range(len(train))
trainplot = ax.plot(xs, train, 'r-', label = 'training accuracy')
validplot = ax.plot(xs, valid, 'g-', label = 'validation accuracy')
ax.set_xlabel('epoch')
ax2 = ax.twinx()
doaplot = ax2.plot(xs, doa, 'b-', label = 'validation DoA error')


# added these three lines
lns = trainplot+validplot+doaplot
labs = [l.get_label() for l in lns]
ax.legend(lns, labs, loc=0)

 

plt.show()
    