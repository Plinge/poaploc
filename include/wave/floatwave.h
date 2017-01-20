#ifndef FLOATWAVE_H
#define FLOATWAVE_H
#pragma once

#include "wave/wave.h"

namespace Audio {

/**
 * @ingroup audio
 * @brief floating point multichannel audiodata.
 *
 * @author Axel Plinge
 *
 * @par history
 *  -  15.06.2012  Axel Plinge  created
 */

#define WAVE_TYPE_FLOATWAVE 0xF

class FloatWave : public Wave
{
protected:
    float  samplingRate;
    int    channels;
    int    sampleCount;
    float* data;
    long datasize;
    static const int WAVE_TYPE = WAVE_TYPE_FLOATWAVE;


    void copy(const Wave & other);

public:
    FloatWave();

    FloatWave(float fs,unsigned int c,unsigned int n);

    FloatWave(const Wave & other)
        : data(0), datasize(0)
    {
        copy(other);
    }

    virtual ~FloatWave();

    const Wave& operator = (const Wave&other)
    {
        copy(other);
        return *this;
    }

    virtual int getTypeNumber() const
    {
        return WAVE_TYPE;
    }

    virtual bool isSparse() const
    {
        return false;
    }

    virtual bool isInfinite() const
    {
        return false;
    }

    virtual void create(double f, int ch, int s,bool dirty=false);

    inline int getDataSize() {
        if (data==0) return 0;
        return datasize;
    }

    float* getPtr()
    {
        return data;
    }

    const float* getConstPtr() const
    {
        return data;
    }

    void setToFloat(FloatWave & other);

    void setTo(Wave & other);

    void setSamples(float* newData,int number,int startIndex=0);

    void setGeometry(long n,int c);

    virtual void setSampleCount(long n);

    virtual void zero();

    inline void zero_inline();

    void calcAverage1(FloatWave & out);

    virtual void multiplyBy(double v, int channel);

    virtual double getSample(long p,int chan) const;
    
    virtual void setSample(long p,int chan,double v);

    virtual double getSamplingFrequency() const
    {
        return samplingRate;
    }

    virtual int getFrameCount() const
    {
        return sampleCount;
    }

    virtual int getChannels() const
    {
        return channels;
    }

    virtual int getNextNonzeroIndex(long p,int chan, int & hint, double & v) const;

    virtual double getMaxSample(int channel=-1) const;

    virtual int getDataSamplesTotal() const
    {
        return getFrameCount()*getChannels();
    }

    virtual int getDataSamples(int channel) const
    {
        return getFrameCount();
    }

    virtual int getNonzeroSampleCount(int channel=-1);


    virtual void shiftBy(int samples, bool dirty=false);



};

}

#endif // FLOATWAVE_H
