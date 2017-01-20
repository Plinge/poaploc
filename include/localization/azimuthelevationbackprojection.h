#pragma once
#undef max
#undef min
#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>
#include "localization/tabulartwodeebackprojection.h"
#include "micarr/micarray.h"
#include <iomanip>      // std::setprecision

/**
  *  table->at(elevationIndex)[azimuthIndex] = tdoa
  */
class AzimuthElevationBackprojection  : public TablularTwoDeeBackprojection
{
protected:
    double elevationStep;
    double azimuthStep;
    int elevationOffset;
    double samplingFrequency;
    double dmax;
    double c;
    double extraAngularOffset;
    bool half;

public:
    AzimuthElevationBackprojection(double fs, double as=1, double am=360.0, double es=7.0, double ev=90) :
    extraAngularOffset(0),
    dmax(1.5),
    elevationStep(es),  azimuthStep(as), half(false), samplingFrequency(fs)
    {
        elevationOffset = 0;
        if (es>91) {
            elevationOffset = -90;
        }
        dimSup = (int)ceil((double)ev/(double)es);
        if (dimSup<1) dimSup=1;
        dimSub = (int)ceil(am/as);
        samplingFrequency=fs;
        c=343.0;
        half = am <= 180.0;
    }

    double getAzimuthStep()
    {
        return azimuthStep;
    }

    double getElevationStep()
    {
        return elevationStep;
    }

    double azimuth(int a) {
        if (half) {
            return 180.0 - (a*azimuthStep);
        }
        return (a*azimuthStep) - 180.0;
    }

    double elevation(int e) {
        return (e*elevationStep) + elevationOffset;
    }

    virtual void setExtraAngularOffset(double d)
    {
        extraAngularOffset=d;
    }

    inline int getElevationStepCount()
    {
        return dimSup;
    }

    inline int getAzimuthStepCount()
    {
        return dimSub;
    }

    inline int getChannelIndex(int elevation, int azimuth)
    {
        return elevation*dimSub+azimuth;
    }

};




class AzimuthElevationBackprojectionGen : public AzimuthElevationBackprojection
{
public:

public:

    AzimuthElevationBackprojectionGen(double fs,MicArray* m,double dm=1.5,int ev=90,double as=1,double es=5,double am=360.0,double e0=0.0)
        : mic(m), AzimuthElevationBackprojection(fs, as, am, es, ev)
    {
        dmax=dm;
        elevationOffset=e0;
    }

//    AzimuthElevationBackprojectionGen(double fs,double diameter,int count,double dm,int ev=90,double as=1,double es=5) :
//      AzimuthElevationBackprojection(fs,diameter,count,dm,ev,as,es)
//      {

//      }
protected:
	MicArray* mic;


	virtual void init()
    {
        double r=dmax; // well, anything beyond 1.5m is infinte
        int count = mic->getMicrophoneCount();
        DoublePoint3 center =  mic->getCenterPosition();
//        AP_String mps = mic->getMicPosString();
//        LogFmtA("AzimuthElevationBackprojectionGen az = %.0f-%.0f / %.0f el = %.0f-%.0f / %.0f array %d mics %.2fm center %.3f %.3f %.3f :: %s",
//               (double)0.0, (double)(dimSub-1) * azimuthStep, azimuthStep,
//               (double)elevationOffset, (double)elevationOffset+(dimSup-1)*elevationStep, elevationStep,
//                count, mic->getMaxDistance(),
//                center.x, center.y, center.z,
//                (const char*)mps
//                );
        
        double tmin=0.0,tmax=0.0;
        std::cerr;
        //std::cerr << std::fixed;
		for (int i1=0;i1<count;i1++) {
			DoublePoint3 m1 = mic->getMicPosition(i1);
			for (int i2=0;i2<count;i2++) {				
				if (i1!=i2) {
                    DoublePoint3 m2 = mic->getMicPosition(i2);
                    if (i1<i2 && verbose) {
                        double dist = DoublePoint3::distance(m1,m2);
                        double tdoa = dist * samplingFrequency / c;                        
                        std::cerr.precision(5);
                        std::cerr << "pair " << i1 << "," << i2 << " d = "  << dist << " t = "   << tdoa << std::endl;
                    }
                    TimeDelayTable table(getElevationStepCount(),getAzimuthStepCount());
                    for (int elevationIndex=0;elevationIndex<dimSup;elevationIndex++) {
                       for (int azimuthIndex=0;azimuthIndex<dimSub;azimuthIndex++) {
                           DoublePoint3 p = DoublePoint3::spherical(r, -azimuthIndex*azimuthStep  + extraAngularOffset, elevationIndex*elevationStep + elevationOffset);
                           p += center; 
                           double dist1 = DoublePoint3::distance(m1,p);
                           double dist2 = DoublePoint3::distance(m2,p);
                           double dist = dist1 - dist2;
                           double tdoa = dist * samplingFrequency / c;
                           //table->at(elevationIndex)[azimuthIndex] = tdoa;
                           table.setValue(elevationIndex,azimuthIndex,tdoa);
                           if (tdoa<tmin) tmin = tdoa;
                           if (tdoa>tmax) tmax = tdoa;
						}
					}
                    tables.insert(i1*count + i2, table);
				}
			}			
		}
        init_done=true;
//        LogFmtA("AzimuthElevationBackprojectionGen.init done with fs = %f, c = %f, tdoa range  = %f .. %f.", (double)samplingFrequency,  (double)c,  (double)tmin,  (double)tmax);
	}
};
