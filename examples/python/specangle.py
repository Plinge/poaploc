# -*- coding: utf-8 -*-
'''
Created on 21 Feb 2013

@author: aplinge
'''

import math
import re
import numpy as np
import spectra
import angles

class SpecAngle:
    def __init__(self,a=0,s=[],v=10.0,p=1.0,u=1.0,label=None):
        self.a = a
        self.e = 0
        self.s = np.array(s)
        self.v = v
        self.p = p
        self.u = u
        self.ns = None
        self.ss = None
        self.ls = None
        self.label = label
        self.t = None
        self.meta = dict()
        
    def geta(self):            
        return self.a
    
    def seta(self,v):
        self.a=v

    def gete(self):
        return self.e

    def sete(self,v):
        self.e=v

    def gett(self):
        return self.t

    def sett(self,v):
        self.t=v
        
    def getv(self):            
        return self.v
    
    def setv(self,v):
        self.v=v
        
    def getu(self):            
        return self.u
    
    def setu(self,u):
        self.u=u

    def getmeta(self,key,default=None):
        if  not key in self.meta.keys():
            self.meta[key]=default            
        return self.meta[key]

    def setmeta(self,key,u):
        self.meta[key]=u
    
    def gets(self):
        return self.s

    def getlabel(self):
        return self.label

    def setlabel(self,l):
        self.label=l
    
    def __like__(self,ss):
        if ss<=0.0:
            ss=-120.0
        ss=10.0*math.log10(ss)
        ss = (40.0+ss)/60.0
        if ss<0.0:
            ss = 0.0
        if ss>1.0:
            ss = 1.0
        return ss
    
    def getns(self):
        if self.ns==None:
            self.ns = spectra.normalize(self.s)
        return self.ns
    
    def sets(self,v):
        self.ns = None
        self.ls = None
        self.ss = None
        self.s=v
        
    def getsum(self):
        if self.ss==None:
            self.ss = np.sum(self.s)
        return self.ss

    def getls(self):
        if self.ls==None:
            self.ls = [self.__like__(self.s[j])  for j in range(0,len(self.s))]
        return self.ls    
      
    def geten(self):
        ss = self.getsum()
        if ss<=0.0:
            return -120.0
        return 10.0*math.log(ss)
    
    def getlike(self):
        return self.__like__(self.getsum())
    
    def getbands(self,th=0.01):
        n=0
        for i in range(0,len(self.s)):
            if self.s[i]>th:
                n = n + 1
        return n

    def getp(self):
        return self.p

    def setp(self,v):
        self.p=v
        
    def  __str__(self):
        s = ""
        if self.time != None:
            s = "t=%.2f " % self.time
        s += "a=%6.1f %5.1f su=%4d/%2d: " % (self.a, self.v, int(self.getu()), self.getbands())
        s += spectra.specstring(self.getns())
        return s

    angle = property(geta,seta,None,"angle")
    elevation = property(gete,sete,None,"elevation")
    sigma = property(getv,setv,None,"angle  standard deviation")
    spectrum = property(gets,sets,None,"spectrum")
    normspectrum = property(getns,None,None,"normalised spectrum")
    logspectrum = property(getls,None,None,"log spectrum")
    probability = property(getp,setp,None,"probability")
    support = property(getu,setu,None,"support")
    label = property(getlabel,setlabel,None,"label")
    time = property(gett,sett,None,"time")
    
class SAGet:
    def __init__(self,conn,table):
        self.table   = table
        self.cur = conn.cursor()
        query  = 'SELECT min(time),max(time) FROM '+table
        self.cur.execute(query)
        dat = self.cur.fetchone()
        self.tstart = dat[0]
        self.tend   = dat[1]        
        query = 'SELECT * FROM ' + table + ' LIMIT 1'
        self.cur.execute(query)
        colnames = [desc[0] for desc in self.cur.description]
        self.timecol  = colnames.index('time')
        self.anglecol = colnames.index('angle')
        if 'ss' in colnames:
            self.sucol = colnames.index('ss')
        elif 'value' in colnames:
            self.sucol = colnames.index('value')
        else:
            self.sucol  = None                    
        if 'dev' in colnames:        
            self.devcol = colnames.index('dev')
        else:
            self.devcol = None
        if 'label' in colnames:
            self.labelcol = colnames.index('label')
        else:
            self.labelcol = None
        if 'time' in colnames:
            self.timecol = colnames.index('time')            
        else:
            self.timecol = None
            print "no time column?"
        if 'elevation' in colnames:
            self.elevationcol = colnames.index('elevation')
        else:
            self.elevationcol = None
        self.s0col    = colnames.index('s0')
        self.sncol    = self.s0col
        while re.match('s\d+',colnames[self.sncol+1]):
            self.sncol=self.sncol+1
            if self.sncol+1==len(colnames):
                 break
        print "%s %d bands" % (table,self.getnbands())         



    def gettimerange(self):
        return [self.tstart,self.tend]
    
    def getnbands(self):
        return self.sncol-self.s0col+1
        
    def fetch(self,time,window,xxx='',nb=0,ss=0,justnewest=False):
        query = 'SELECT * FROM ' + self.table + ' WHERE ABS(time-' +  str(time) +')<'+str(window*0.5)+' '+xxx
        #print query
        self.cur.execute(query)
        res = []
        firsttime=None
        for row in self.cur.fetchall():            
            a = SpecAngle(row[self.anglecol], [row[x] for x in range(self.s0col,self.sncol+1)], 5.0)
            if a.getbands()>nb and a.getsum()>ss:
                if self.devcol != None:
                    a.setv(row[self.devcol])                
                if self.sucol != None:
                    a.setu(row[self.sucol])
                else:
                    a.setu(a.getsum())
                if self.labelcol != None:
                    a.setlabel(row[self.labelcol])
                if self.timecol != None:
                    a.sett(row[self.timecol])
                if self.elevationcol !=  None:
                    a.sete(row[self.elevationcol])
                #print "read " + str(a) + "  time  " + str(a.time)
                if firsttime is None:
                    firsttime=a.time
                if justnewest and a.time - firsttime >0.1:
                    break      
                res.append(a)
        
        return res

    def average_angle(self,time,window,xxx=''):
        query = 'SELECT angle,ss FROM ' + self.table + ' WHERE ABS(time-' +  str(time) +')<'+str(window*0.5)+' '+xxx
        self.cur.execute(query)
        data = self.cur.fetchall()
        #print query
        #print data
        if len(data)<1:
            return None
        x,y = zip(*data)
        return angles.weightedAverage(x,y)
        
    def clear(self):        
        self.cur.close()    
