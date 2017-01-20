#pragma once	
#include "geometry/doublept.h"
#include <map>
#include <list>
#include <QVector>


class MicArray {

public:
	virtual DoublePoint3 getMicPosition(int i)=0;
	virtual int getMicrophoneCount()=0;
 
	virtual double getMicDistance(int i,int j=0)
	{
		DoublePoint3 pi = getMicPosition(i);
		DoublePoint3 pj = getMicPosition(j);
		return pi.distance(pj);
	}

	virtual bool haveMicAt(double x,double y,double dx,double dy=-1.0)
	{
		if (dy<0) dy=dx;
		int n = getMicrophoneCount();
		for (int index=0;index<n;index++) {
			DoublePoint3 p = getMicPosition(index);
			if (p.x>=x && p.x<=(x+dx) && p.y>=y && p.y<=(y+dy)) {
				return true;
			}
		}
		return false;
	}

    virtual double getMaxDistance()
    {
        double m = 0.0;
        int n = getMicrophoneCount();
        for (int index=0;index<n;index++) {
            DoublePoint3 p = getMicPosition(index);
            for (int index2=index+1;index2<n;index2++) {
                DoublePoint3 p2 = getMicPosition(index2);
                double d  =  p.distance(p2);
                if (d>m) m=d;
            }
        }
        return m;
    }

    virtual bool isLinear()
    {
        int n = getMicrophoneCount();
        if  (n<3) return true;

        bool linear = true;
        for (int i=0;i<=n-3;i++) {
            DoublePoint3 p1 = getMicPosition(i+0);
            DoublePoint3 p2 = getMicPosition(i+1);
            DoublePoint3 p3 = getMicPosition(i+2);
            DoublePoint3 d1 = p1-p2;
            DoublePoint3 d2 = p2-p3;
            double test = DoublePoint3::cross(d1,d2).length();
           // cout << test  << ", ";
            if (fabs(test) > 1e-7) linear=false;

        }
        //cout << endl;
        return linear;
    }
    
    DoublePoint3 getCenterPosition()
    {
        DoublePoint3 c(0.0,0.0,0.0);
        for (int i=0;i<getMicrophoneCount();i++) {
            c += getMicPosition(i);
        }
        c *= 1.0 /  getMicrophoneCount();
        return c;        
    }
    
    inline int getHash(int index1,int index2)
    {
        return index1*getMicrophoneCount()+index2;
    }

    inline void decodeHash(int hash,int & index1, int &  index2)
    {
        int n=getMicrophoneCount();
        index1 = hash / n;
        index2 = hash - n*index1;
    }

    int getMicDistanceHash(int index1,int index2)
    {
        return static_cast<int>(0.5+1000.0*fabs(getMicDistance(index1,index2)));
    }

    typedef std::list<int> distgroup;
    typedef std::map<int,distgroup> distgroups;

    virtual distgroups getDistanceGroups()
    {
        distgroups res;
        for (int index1=0;index1<getMicrophoneCount();index1++) {
            for (int index2=index1+1;index2<getMicrophoneCount();index2++) {
                int distance = getMicDistanceHash(index1,index2);
                if (res.find(distance) == res.end()) {
                    res[distance] = distgroup();
                }
                res[distance].push_back(getHash(index1,index2));
            }
        }
        return res;
    }
    
};


class GenericMicArray : public MicArray
{
protected:
    QVector<DoublePoint3> positions;

public:
    virtual DoublePoint3 getMicPosition(int i)
    {
        return positions[i];
    }

    virtual int getMicrophoneCount()
    {
        return positions.count();
    }


};

class KinectMicArray : public GenericMicArray
{

public:
    KinectMicArray()
    {
        positions.append(DoublePoint3(-0.113,0.0,0.0));
        positions.append(DoublePoint3( 0.036,0.0,0.0));
        positions.append(DoublePoint3( 0.076,0.0,0.0));
        positions.append(DoublePoint3( 0.113,0.0,0.0));
    }

    virtual bool isLinear() {
        return true;
    }
};


class ArbitraryMicArray : public GenericMicArray
{
public:
    virtual void add(DoublePoint3 p)
	{
        positions.append(p);
	}

};
