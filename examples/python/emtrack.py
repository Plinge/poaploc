# -*- coding: utf-8 -*-
'''
Created on 21 Feb 2013

@author: aplinge
'''

import math
import angles 
import spectra
import specangle
import itertools
import numpy as np
from angles import NormalDevAngles
from emdefs import *

class SpecAngleEM:
    def __init__(self,n=0,data=None,mode=0):
        self.like = 0
        self.mode=mode
        self.verbouse=0
        self.init(n,data)              
        
    def init(self,n=0,data=None):
        self.densities=[]
        self.spectra=[]        
        self.weight=[]
        self.probs=np.zeros((1,n))
        self.energy=[]
        self.datasp=[]        
        if n>0:
            if n>len(data):
                n=len(data)

        for i in range(0,n):
            self.spectra.append(np.array(data[i].normspectrum))
            
        for i in range(0,n):
            self.densities.append(NormalDevAngles(data[i].angle,99.0))            
            self.weight.append(1.0/n)
            self.energy.append(0.1)
        
        self.updateData(data)
           
    def updateData(self,data=None): 
        if not data:
            return
        self.datarange = range(0,len(data))                        
        self.dataan = [data[j].angle for j in self.datarange]
        self.sj = np.array([data[j].getsum() for j in self.datarange])
        self.sj = self.sj * (len(data) / np.sum(self.sj))
        self.sj.shape = (1,len(data))
        self.datasp = [data[j].normspectrum for j in self.datarange]
                                             
    def setverbousity(self,v):
        self.verbouse=v
        
    def getmodelprobs(self):
        n = 360
        mog=np.zeros((n,))
        for j in range(n):
            a = j-180
            pp = 0.0
            for i,(w,dens) in enumerate(zip(self.weight,self.densities)):
                pp += w * dens.prob(a)
            mog[j]=pp        
        return mog
        
    def prob_i_x_spec(self,i,j):
        p = self.weight[i] * self.densities[i].prob(self.dataan[j]) * spectra.correlation_n(self.datasp[j], self.spectra[i]) #* x.getsum()
        #print "p(%5.1f|%d) = .. %f .. = %f" % (x.angle, i, self.densities[i].prob(x.angle), p)
        return p

    def prob_i_x_doa(self,i,j):
        p = self.weight[i] * self.densities[i].prob(self.dataan[j]) 
        #print "p(%5.1f|%d) = .. %f .. = %f" % (x.angle, i, self.densities[i].prob(x.angle), p)
        return p
        
    def getangles(self):
        return map(lambda i : self.densities[i].getmu(), range(0,len(self.densities)))

    def getenhist(self,data):
        h = np.zeros((360,1))
        for x in data:
            a = x.angle
            s = x.getsum()
            h[a+180]+=s
        return h
        
    def getn(self):
        return len(self.densities)
    
    def __fill__(self):
        n = self.getn()
        while len(self.energy)<n:
            self.energy.append(0)
        if len(self.probs)<n:
            self.probs.resize((1,n))
        
    def getcentroid(self,i):
        p=0
        if len(self.probs)>i:
            p=self.probs[i]
        u=0
        if len(self.energy)>i:
            u=self.energy[i]
        return specangle.SpecAngle(self.densities[i].mu, self.spectra[i], self.densities[i].sigma, self.weight[i], u)
    
    def getcentroids(self):
        #self.__fill__()
        ret = []
        for i in range(0,len(self.densities)):
            ret.append(self.getcentroid(i))
        return ret
    
    def centroid_str(self,i):
        s = "p(%d) = %3.1f " % (i,self.weight[i])
        s = s + str(self.getcentroid(i))
        return s

    def printcentroids(self):
        for i in range(0,len(self.densities)):
            print self.centroid_str(i)

    def printem(self):
        pi = np.sum(self.p,1)
        for i in range(0,len(self.densities)):
            p = pi[i]
            if p>0.0:
                print "%s sum p(%d|x) = %g" % (self.centroid_str(i), i, p)
            else:
                print "%s sum p(%d|x) = 0!" % (self.centroid_str(i), i)

    def remove(self,i,quiet=False,reason=""):
        if self.verbouse>0 and not quiet:
            print "removed class ", i, reason
            print self.centroid_str(i)
        del self.weight[i]
        del self.densities[i]
        del self.spectra[i]
 
    def expect(self,data):
        # update p(i | x_n, t_i)
        #print "mode %d" %  self.mode
        if self.mode == EM_USE_SPECTRUM_SUMLIKE:
            pfun = self.prob_i_x_spec
        else:
            pfun = self.prob_i_x_doa
           
        self.p = np.zeros((len(self.densities),len(data)))
        for i,j in itertools.product(range(0,len(self.densities)), range(0,len(data))):
            self.p[i,j] = pfun(i,j)
        
        self.like  = np.sum(self.p)
        self.probs = np.sum(self.p,1)
        
        if self.getn()>1:
            pj = np.sum(self.p,0)
            self.p = self.p / pj    
            if self.mode != EM_USE_DOA_ONLY:       
                self.p = self.p * self.sj                        
        else:
            if self.mode != EM_USE_DOA_ONLY:
                self.p = np.array(self.sj)
            else:
                self.p = np.ones( (1,len(data)) ) / float(len(data))
                
        self.p.shape = (self.getn(),len(data))
                 
        if self.verbouse>2:            
            self.printem()
            
        return self.like

    def maximize(self,data):
        drange = range(0,len(data))            
        pi = np.sum(self.p, 1)
        si = []
        removals=[]
        i= 0
        for i in range(len(self.densities)):            
            si.append(pi[i])            
            if pi[i]>0:
                angles = [data[j].angle for j in drange]
                yi = [(self.p[i,j] / float(pi[i])) for j in drange]
                # sum yi == 1                
                self.densities[i].update(angles, yi)
                specs  = [data[j].normspectrum for j in drange]                
                self.spectra[i] = spectra.weightedAverage(specs, yi)                
                en=0
                for (s,y) in itertools.izip((self.sj[0,j] for j in drange),(self.p[i,j]  for j in drange)):
                    en = en+s*y
                self.energy[i] = en #* len(data)
            else:
                removals.append(i)
        
                        
        if len(self.densities)==1:            
            self.weight[0] = 1.0
        elif len(self.densities)>1:
            ss = sum(si)
            if ss>0:
                for i in range(0,len(self.densities)):
                    self.weight[i] = si[i] / ss
            else:
                for i in range(0,len(self.densities)):
                    self.weight[i] = 1.0 / len(self.densities)
    
        #for i in range(0,len(self.densities)):
        #    self.energy[i] = self.energy[i] * self.weight[i]
        #if self.verbouse>2:
        #self.printem()
        for i in removals[::-1]:            
            self.remove(i,'p~0')
    
    def remove_small(self,delthreshold):
        nremoved=0
        i=0
        while i<len(self.densities):
            if self.getn()<2:
                break;
            if self.weight[i] < delthreshold:
                self.remove(i, False, ("w =%g" % self.weight[i]) )
                nremoved=nremoved+1
            else:
                i=i+1
        return nremoved

    def remove_small_support(self,delthreshold):
        nremoved=0
        i=0
        while i<len(self.densities):
            if self.getn()<2:
                break;
            if self.energy[i] < delthreshold:
                self.remove(i, False, ("su=%g" % self.energy[i]) )
                nremoved=nremoved+1
            else:
                i=i+1
        return nremoved

    def remove_tight(self,delthreshold):
        nremoved=0
        i=0
        while i<len(self.densities):
            if self.getn()<2:
                break;
            if self.densities[i].sigma < delthreshold:
                self.remove(i, False, ("sigma = %f" % self.densities[i].sigma))
                nremoved=nremoved+1
            else:
                i=i+1
        return nremoved
    
    def join_close(self,jointhreshold,mincor=0.5):
        njoined=0
        i=0;
        while i<len(self.densities):
            if self.getn()<2:
                break;
            j=i+1
            while j<len(self.densities):
                dd = abs(angles.difference(self.densities[i].mu, self.densities[j].mu))
                ds = spectra.correlation(self.spectra[i], self.spectra[j])
                if dd < jointhreshold and ds>mincor:
                    if self.verbouse>0:
                        print "joined classes d = %f s =%f %s & %s" % (dd, ds,  self.centroid_str(i) ,self.centroid_str(j))
                    self.densities[i].set(angles.average([self.densities[i].mu,self.densities[j].mu]), 0.5*(self.densities[i].var+self.densities[j].var))
                    self.remove(j,True)
                    njoined=njoined+1
                else:
                    j=j+1
            i=i+1    
        return njoined
    
    def split_wide(self,splitthreshold):
        nsplit=0
        for i in range(0,len(self.densities)):
            if self.densities[i].sigma > splitthreshold: # and self.densities[i].sigma < 180.0:
                if self.verbouse>0:
                    print "split class", self.centroid_str(i)
                s1 = self.densities[i].sigma
                v2 = math.pow(s1,2.0)
                m1 = angles.make180(self.densities[i].mu - s1)
                m2 = angles.make180(self.densities[i].mu + s1)
                self.densities[i].set(m1,v2)
                self.densities.append(NormalDevAngles(m2,v2))
                self.spectra.append(self.spectra[i])
                self.weight.append(self.weight[i]*0.5)
                self.weight[i]=self.weight[i]*0.5
                self.energy.append(self.energy[i]*0.5)
                self.energy[i]=self.energy[i]*0.5
                                
                if self.verbouse>0:
                    print "new class", self.centroid_str(i)
                    print "new class", self.centroid_str(self.getn()-1)

                nsplit=nsplit+1
                                   
        return nsplit                

    def fitstep(self,data,it,change,threshold,delthreshold,splitthreshold,jointhreshold,maxiter,miniter):
        # check if we need to change the number of classes
        if it>2 and change<4:
            if self.join_close(jointhreshold)>0:
                change = 6
            #change = change + self.remove_small_support(1.0)
            #change = change + self.remove_tight(2.5)
            if self.remove_small(delthreshold)>0:
                change = 6
                
        if it>2 and change<3 and it<maxiter-3:
            if self.split_wide(splitthreshold)>0:
                change = 6
        # calculate  probabilities
        self.expect(data)
        # check if we converged enough
        if it>miniter and change==0:
            d = self.like-self.ll
            if d<threshold:
                return -1
        self.ll=self.like
        # no? maximize!
        self.maximize(data)
        
        if self.getn()<1:
            return -1
        
        return change
    """
    
    em.fit(res,
    threshold=1e-6
    delthreshold=1e-2
    splitthreshold=22.0 .. ? 15
    jointhreshold=12.0 .. ? 5
    -- em.fit(s,threshold=1e-6,delthreshold=1e-2,splitthreshold=22.0,jointhreshold=12.0,maxiter=20)
    """
    def fit(self,data,threshold=1e-2,delthreshold=1e-2,splitthreshold=15.0,jointhreshold=5.0,maxiter=50,miniter=5):        
        if self.getn()<1:
            self.init(1,data)
        else:
            self.updateData(data)
            
        it=0; change = 0; self.ll=0
        if self.verbouse>1:
            s = 'data = '
            for x in data:
                s = s +  ("%.0f (%.1f) " % (x.angle, x.getsum()))
            print s
            
        while True:
            change = self.fitstep(data,it,change,threshold,delthreshold,splitthreshold,jointhreshold,maxiter,miniter)            
            # check if we iterated too long or converged
            if change<0:
                break
            it=it+1
            if it>maxiter:
                break       
            if self.verbouse>2:
                print "%d iterations, change=%d, like = %g" % (it, change, self.like)
            if self.verbouse>0 and self.verbouse<=2 and change>0:            
                print "%d iterations, change=%d, like = %g" % (it, change, self.like)
            if self.verbouse>0 and self.verbouse<=2 and change>0:
                self.printem()                                    
            if change>0:
                change = change - 1
            
        # loop done     
                   
        if self.verbouse>1:
            print "%d iterations, like = %g" % (it, self.like)
            self.printem()
            
        return self.like
