import numpy as np
import specangle

def anglesFromMsl(alldata, time, window, threshold=0, bands=16):
    """
    return a list of SpecAngles from a msl data for the given time +- window
    """
    sdata = alldata[ (alldata[:,1] >= time - window) & (alldata[:,1] < time + window) & (alldata[:,4]>=threshold)  ]
    res = []
    for row in sdata:
        spec = [row[x] for x in range(5,5+bands)]
        if np.sum(spec)<=0: 
            continue
        a = specangle.SpecAngle(row[2], spec, 5.0)
        a.setu(row[4])
        a.sete(row[3])
        a.time = row[1]
        res.append(a)
    return res

def timespanFromMsls(data):
    ts = np.floor(np.min(data[0][:,1]))
    te = np.ceil(np.max(data[0][:,1]))
    for i in range(1,len(data)):
        ts = min(ts, np.floor(np.min(data[0][:,1])))
        te = max(te, np.ceil(np.max(data[0][:,1])))
    return ts,te
    
def anglesFromMsls(data, time, window, threshold=0):
    """
    return a list of SpecAngles from a msl data for the given time window
    """
    allres=[]
    for alldata in data:
        sdata = alldata[ (alldata[:,1] >= time - window) & (alldata[:,1] < time + window) & (alldata[:,4]>=threshold)  ]
        res = []
        for row in sdata:
            spec = [row[x] for x in range(5,5+16)]
            a = specangle.SpecAngle(row[2], spec, 5.0)
            a.setu(row[4])
            a.sete(row[3])
            res.append(a)
        allres.append(res)
    return allres

def clusteredAngleToRow(frame, time, x):
    row = [frame, time, x.angle, x.elevation, x.getsum(), x.getp(), x.sigma ]
    row.extend(x.spectrum)
    return row

def labeledAngleToRow(frame, time, label, x):
    row = [frame, time, label, x.angle, x.elevation, x.getsum(), x.getp(), x.sigma ]
    row.extend(x.spectrum)
    return row

def anglesFromEM(data, time, window, threshold=0.05):
    """
    return a list of SpecAngles from a msl data for the given time window
    """
    allres=[]
    for alldata in data:
        sdata = alldata[ (alldata[:,1] >= time - window) & (alldata[:,1] < time + window) & (alldata[:,4]>=threshold)  ]
        res = []
        for row in sdata:
            spec = row[7:]
            a = specangle.SpecAngle(row[2], spec, 5.0)
            a.setu(row[4])
            a.sete(row[3])
            a.setp(row[5])
            a.sigma=row[6]
            a.time=row[1]
            res.append(a)
        allres.append(res)
    return allres