#pragma once
#include <sstream>
#include "stdio.h"
#include <math.h>

class AngularDetection
{
    static const int MAX_BANDS=32;
public:    
    float x,y,z; //< origin
    float azimuth,elevation;
    float variance;
    float spectrum[MAX_BANDS];
    float probability;
    int label;

public:    
    AngularDetection() :
    x(0.0F),y(0.0F),z(0.0F), azimuth(0.0F),elevation(0.0F), variance(0.0F), probability(1.0F), label(0)
    {
        for  (int i=0;i<MAX_BANDS;i++) {
            spectrum[i]=0.0F;
        }        
    }

    void setSpectralValue(int i,float v)
    {
        //assert(i<MAX_BANDS);
        spectrum[i]=v;
    }

    inline float getSpectralValue(int i) const
    {
        //assert(i<MAX_BANDS);
        return spectrum[i];
    }

    inline float getSpectralSum() const
    {
        float s=0.0F;
        for (int i=0;i<MAX_BANDS;i++) {
            s+=spectrum[i];
        }
        return s;
    }

    inline float getProbability() const
    {
        return probability;
    }

    inline void setProbability(float p)
    {
        probability=p;
    }

    inline int getLabel() const
    {
        return label;
    }

    inline void setLabel(int l)
    {
        label=l;
    }

    std::string toInfoString()
    {
        char  buf[200];
        if (variance>0.0F) {
            sprintf(buf,"#%d %.1f,%.1f +- %.1f l=%.3f", label, azimuth,  elevation,  sqrt(variance), probability);
        } else {
            sprintf(buf,"#%d %.1f,%.1f l=%.3f", label, azimuth,  elevation, probability);
        }
    }

    inline double getAngle() const
    {
        return azimuth;
    }

    inline double getElevation() const
    {
        return elevation;
    }
};

