#pragma once
#undef max
#undef min
#define _USE_MATH_DEFINES
#include <math.h>
#include "APTL/aplogfmt.h"
#include "audio/AP_SigProc.h"
#include "audio/AP_wav.h"
#include <algorithm>
#include "micarr/circularmicarray.h"
#include "micarr/tabulartwodeebackprojection.h"
 

class AngleDistanceBackprojection : public TablularTwoDeeCircArrBackprojection
{
protected:
	double maxDistance;
	double minDistance;

    CircularMicArray mics;

	virtual DoublePoint getPosition(double a,double r)
	{
		double x = 	sin( (a * M_PI) / 180.0 ) * r;
		double y = 	cos( (a * M_PI) / 180.0 ) * r;
		return DoublePoint(x,y);
	}

	void init(double samplingFrequency,double dmax=2.5,double dmin=-1.0,double c=343.0)
	{
		
		
		if (dmin <0.0) {
			minDistance = ceil(mics.getRadiusM()*10)/10.0;
		} else {
			minDistance = ceil(dmin*10)/10.0;
		}
		maxDistance = dmax;
		LogFmtA("AngleDistanceBackprojection.init %.2f-%.2fm array d = %.2fm",minDistance,maxDistance,mics.getRadiusM()*2.0);
		dim1 = 1+floor((maxDistance-minDistance)*10);
		dim2 = 360;
		int count = mics.getMicrophoneCount();
		for (int i1=0;i1<count;i1++) {
			DoublePoint m1 = mics.getMicPosition(i1);
			for (int i2=0;i2<count;i2++) {				
				DoublePoint m2 = mics.getMicPosition(i2);
				if (i1!=i2) {					
					TimeDelayTable* table = new TimeDelayTable(dim1,dim2);
					for (int d=0;d<dim1;d++) {
						double r = d*0.1 + minDistance;
						for (int a=0;a<dim2;a++) {
							DoublePoint p = getPosition(-a,r); // NOTE: angle reversed to fit documentation
							double dist = DoublePoint::distance(m1,p) - DoublePoint::distance(m2,p);
							double tdoa = dist * samplingFrequency / c;
							table->at(d)[a] = tdoa;
						}
					}
					tables.Insert(i1*count + i2, table);
				}				
			}			
		}
        init_done=true;
	}


public:
	/**
 	 * @param diameter Array Diameter in m
	 */	
	AngleDistanceBackprojection(double samplingFrequency,double diameter,int count,double dmax=2.5,double dmin=-1.0) 
		: TablularTwoDeeCircArrBackprojection(diameter,count)
	{
		init(samplingFrequency,dmax,dmin);
	}

	double getDistance(int s)
	{
		return (s*0.1) + minDistance;
	}	

	int getDistanceSteps()
	{
		return dim1;
	}
	


};