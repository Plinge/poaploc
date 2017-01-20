#ifndef POAPLOCALIZERCIRC_H
#define POAPLOCALIZERCIRC_H
#pragma once
#include "micarr/poaplocalizer.h"

class PoAPLocalizerCircular : public PoAPLocalizer
{
protected:
    CircularMicArrayCorrelator mic;
    AzimuthElevationBackprojectionCircArray* backProCirc;
public:
    PoAPLocalizerCircular();
    virtual ~PoAPLocalizerCircular();

    virtual void setMic(double r,int n=8,double a=0.0);
    virtual void setCombination(CircularMicArrayCorrelator::CombinationStrategy sel, double g=0.3);
    virtual int  getMicrophoneCount();
    virtual int  getAngularCount();

protected:
    virtual void correlatePairs(int filterBand, int offset, int step, Audio::WavGroup &correlates,int start=0,int end=8 );
    virtual void correlate(bool continued);
    virtual void initBackprojection();
    virtual AzimuthElevationBackprojection* getBackProjection();
};

#endif
