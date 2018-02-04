import numpy as np

def perp(a) :
    b = np.empty_like(a)
    b[0] = -a[1]
    b[1] = a[0]
    return b

def seg_intersect(a1, da, b1, db):
    """
    compute intersection of directed lines.
    @return: intersection point, None if they do not intersect. 
    """
    dba = np.array(a1) - np.array(b1)
    da_perpendicular = perp(da)
    denom = np.dot(da_perpendicular, db)
    if denom == 0.0:
        return None
    dist_b = (np.dot(da_perpendicular, dba) / denom)
    if dist_b<0:
        return None
    xx = dist_b*db + b1
    
    dab = np.array(b1) - np.array(a1)
    db_perpendicular = perp(db)
    denom = np.dot(db_perpendicular, da)
    if denom == 0.0:
        return None
    dist_a = (np.dot(db_perpendicular, dab) / denom)
    if dist_a<0:
        return None
    #xx == dist_a*da + a1
    return xx


def line_intersect(a1, da, b1, db):
    """
    compute intersection of infinite lines.
    @return: intersection point 
    """
    dp = np.array(a1) - np.array(b1)
    dap = perp(da)
    denom = np.dot(dap, db)
    num = np.dot(dap, dp)
    return abs(num / denom)*db + b1


def avect(a):
    return np.array( [ np.cos(a*np.pi/180.0), np.sin(a*np.pi/180.0) ] )

def ang_intersect(p1, a1, p2, a2):
    return seg_intersect(p1, avect(a1), p2, avect(a2))

def goodness(a1,a2):
    """
    intersection quality
    """
    return abs(np.sin((a1-a2)*np.pi/180.0 ))


if __name__ == "__main__":
    
    import matplotlib.pyplot as plt
    fig = plt.figure()

    for i,(a1,a2) in enumerate(zip([55.7,60.7],[56.2,60.8])):    
        ax   = fig.add_subplot(1,2,1+i)
        p1 = np.array([3.30, 1.845])
        p2 = np.array([2.77, 1.056])
        px  = ang_intersect(p1,a1,p2,a2)
        ps = np.vstack((p1,p2,px))
        
        ax.plot(ps[:,0],ps[:,1],'kx')
        v = avect(a1)
        ax.plot([p1[0],p1[0]+v[0]*5],[p1[1],p1[1]+v[1]*5],'r-')
        v = avect(a2)
        ax.plot([p2[0],p2[0]+v[0]*5],[p2[1],p2[1]+v[1]*5],'r-')
        
        ax.set_xlim([0,10])
        ax.set_ylim([0,4])

    plt.show()

            
