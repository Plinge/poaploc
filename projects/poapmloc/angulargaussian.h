#ifndef ANGULARGAUSSIAN_H
#define ANGULARGAUSSIAN_H
#include "angulardetection.h"

inline double make180(double x)
{
    while (x >  180.0)  x = x-360.0;
    while (x < -180.0)  x = x+360.0;
    return x;
}

inline double angleDiffernece(double a, double b)
{
    return abs(make180(a-b));
}


class AngularGaussian : public AngularDetection
{
protected:
    float weight;
    float denom;
    int bandCount;
    float time;

public:

    AngularGaussian() :
          weight(1.0F), bandCount(16)
    {

    }

    AngularGaussian(const LocalizedSpectrum & s)
        :  weight(1.0F)
    {
        bandCount= s.getBandCount();
        for (int b=0;b<bandCount;b++) {
            spectrum[b] = s.getSpectralValue(b);
        }
        time = s.getTime();
        azimuth = s.angle;
        elevation = s.elevation;
        if (variance>0.0) {
            denom = 1.0 / sqrt(variance*M_PI*2.0);
        } else {
            denom = 1.0;
        }
    }


    void updateTime(double t) {
        time=t;
    }

    void setWeight(double w) {
        weight=w;
    }

    inline float getWeight()  const {
        return weight;
    }

    void updateAngle(double a,double v,double e = -999) {
        azimuth=a;
		if (v<0.1) {
			v=0.1;
		}
        variance=v;
        if (e>-900) {
            elevation=e;
        }
        if (variance>0.0) {
            denom = 1.0 / sqrt(variance*M_PI*2.0);
        } else {
            denom = 1.0;
        }
    }

    void updateSpectrum(double *s) {
        for (int i=0;i<bandCount;i++) {
            spectrum[i]=s[i];
        }
    }

    inline float getTime()  const
    {
        return time;
    }

    inline int getBandCount()  const
    {
        return bandCount;
    }

    inline int getNonzeroBandCount() const
    {
        int n=0;
        for (int i=0;i<bandCount;i++)    {
            if (spectrum[i]>1e-9) n++;
        }
        return n;
    }

    inline double getVariance()  const
    {
        return variance;
    }

    double spectralCorrelation(const LocalizedSpectrum & other) const
    {
        double sc=0.0;
        for (int index=0;index<bandCount;index++) {
            sc += spectrum[index] * other.spectrum[index];
        }
        return sc;
    }

    double prob(const LocalizedSpectrum & other) const
    {
        double p = weight;
        p *= spectralCorrelation(other);
        double d = angleDiffernece(azimuth,other.angle);
        p *= exp(-d*d / (2.0*variance)) * denom;
        return p;
    }

//    bool init(std::string s)
//    {
//        if (!AngularDetection::init(s)) {
//            return false;
//        }
//        const char* buf = s.c_str();
//        weight=-1.0F;
//        getValue(buf,'w',weight);
//        return true;
//    }

//    std::string toString()
//    {
//        std::stringstream buf;
//        buf  << 'A'
//             << 'x' << x <<  'y' << y << 'z' << z
//             << 'a' << azimuth;
//        if (variance>0.0) {
//            buf << 'v' << variance;
//        }
//        if (weight>=0.0) {
//            buf << 'w' << weight;
//        }
//        buf  << 'p' << probability
//             << 'l' << label;
//        if (annotation[0]!=0) {
//            buf << 't' << annotation;
//        }
//        return buf.str();
//    }

    std::string toInfoString()
    {
        char  buf[200];

        sprintf(buf,"%.2f %.1f +- %.1f l=%.3f", weight, azimuth,  sqrt(variance), probability);

        return std::string(buf);
    }


};


#endif // ANGULARGAUSSIAN_H

