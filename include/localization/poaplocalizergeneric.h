#ifndef POAPLOCALIZERCIRC_H
#define POAPLOCALIZERCIRC_H
#pragma once
#include "localization/poaplocalizer.h"
#include <list>

class PoAPLocalizerGeneric : public PoAPLocalizer
{
protected:
    MicArray* mic;
    AzimuthElevationBackprojection* backPro;
    double azimuthMax;
    int nextCenterOffset,firstCenter;
public:
    PoAPLocalizerGeneric();
    virtual ~PoAPLocalizerGeneric();

    virtual void setMic(MicArray* m);
    virtual void setCorrelationFraming(double len_ms, double step_ms);
    virtual void setCombination(double g=0.3);
    virtual int  getChannelCount();

    virtual void precalc();
    virtual void calc(const float* data, unsigned int frames, bool continued);
    virtual bool getLocalizedSpectra(std::list<LocalizedSpectrum> & res);
    virtual void setInputFrameCount(int n);

    void setMA(double ma)
    {
        azimuthMax=ma;
    }

    DoublePoint3 getMicrophoneCenter() {
        return mic->getCenterPosition();
    }


	double getMaxOutTime();

    virtual int getOutputMicPairCount() const
    {
        if (combination!=0) return 1;
        return pairCount;
    }

protected:
    virtual void correlate(bool continued);
    virtual void initBackprojection();
    virtual void initCorrelation();
    virtual AzimuthElevationBackprojection* getBackProjection();
    inline int getCorrelatorIndex(int band, int index1,int index2)
    {
        if (index1>=index2) {
            throw std::runtime_error("illeagal microphone pair!");
        }
        int micCount = mic->getMicrophoneCount();
        int maxx = ((micCount-1) * (micCount));
        return band * maxx + index1 * micCount + index2;
    }
};

#endif
