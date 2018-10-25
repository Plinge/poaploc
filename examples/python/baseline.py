'''
Created on 12.04.2018

@author: pli
'''
from sklearn.ensemble import RandomForestClassifier
#from sklearn.svm import SVC
from sklearn.ensemble import RandomForestRegressor
import numpy as np

class RaFoClLocalizerSingleDoA(object):
    def __init__(self):
        self.clf =  RandomForestClassifier(100)
            
    def train(self,xx,yy):
        a = np.argmax(xx, 1)
        a = a.reshape(a.shape[0],-1)
        self.clf.fit(a,yy)
    
    def localize(self,x):
        a = np.argmax(x, 0).flatten()
        return self.clf.predict(a)[0]

class RandomForestRegressorSingleDoA(object):
    def __init__(self,n=100):
        self.clf =  RandomForestRegressor(n)
            
    def train(self,xx,yy):
        a = np.array(xx)#np.argmax(xx, 1)
        a = a.reshape(a.shape[0],-1)       
        self.clf.fit(a,yy)
    
    def localize(self,x):        
        a = np.array(x)#np.argmax(xx, 1)
        a = a.flatten()
        return self.clf.predict(a)[0]

class LocalizerMultiDoA(object):
    def __init__(self,steps):
        #self.clf =  [RandomForestClassifier(100) for _ in range(steps)]
        #self.clf = [SVC() for _ in range(steps)]
        self.clf = [RandomForestRegressor(100) for _ in range(steps)]
        
    def train(self,xx,yy):
        a = np.argmax(xx, 1)
        a = a.reshape(xx.shape[0], -1)
        for zz,cl in enumerate(self.clf):
            y = np.array(yy==zz).astype(np.int)
            cl.fit(a,y)
            print 'trained',zz
    
    def localize(self,x):        
        res=[]
        a = np.argmax(x, 0).flatten()
        ps=[]
        for z,cl in enumerate(self.clf):
            p = cl.predict(a)[0]
            ps.append(p)
            if p>0.5:
                res.append(z)
        print ps
        return res