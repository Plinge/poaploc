/*
 * Copyright 2010-2015 (c) Axel Plinge / TU Dortmund University
 *
 * ALL THE COMPUTER PROGRAMS AND SOFTWARE ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND.
 * WE MAKE NO WARRANTIES,  EXPRESS OR IMPLIED, THAT THEY ARE FREE OF ERROR, OR ARE CONSISTENT
 * WITH ANY PARTICULAR STANDARD OF MERCHANTABILITY, OR THAT THEY WILL MEET YOUR REQUIREMENTS
 * FOR ANY PARTICULAR APPLICATION. THEY SHOULD NOT BE RELIED ON FOR SOLVING A PROBLEM WHOSE
 * INCORRECT SOLUTION COULD RESULT IN INJURY TO A PERSON OR LOSS OF PROPERTY. IF YOU DO USE
 * THEM IN SUCH A MANNER, IT IS AT YOUR OWN RISK. THE AUTHOR AND PUBLISHER DISCLAIM ALL
 * LIABILITY FOR DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES RESULTING FROM YOUR USE OF THE
 * PROGRAMS.
 */

#include "floatwave.h"
#include "math.h"
// #include <malloc.h>
#include <memory.h>
#include <qdebug.h>
#include <limits>
#include <assert.h>

using namespace Audio;

void FloatWave::copy(const Wave &other)
{
    create(other.getSamplingFrequency(),other.getChannels(),other.getFrameCount(),false);
    if (other.getTypeNumber()==WAVE_TYPE_FLOATWAVE) {
        memcpy(data,((FloatWave*)(&other))->getConstPtr(),datasize*sizeof(float));
    }
    for (long p=0;p<getFrameCount();p++) {
        for (int c=0;c<getChannels();c++) {
            setSample(p,c,other.getSample(p,c));
        }
    }
}

FloatWave::FloatWave() : samplingRate(16000.0), channels(0), sampleCount(0), data(0), datasize(0)
{

}

FloatWave::FloatWave(float fs,unsigned int c,unsigned int n)
    : samplingRate(fs), channels(c), sampleCount(n),  data(0), datasize(0)
{
    create(fs,c,n);
}

FloatWave::~FloatWave()
{
    if (data) {
        free(data);
        data=0;
        datasize=0;
    }
}

void FloatWave::zero()
{
    zero_inline();
}

void FloatWave::zero_inline()
{
    if (data==0) {
        qWarning("FloatWave.zero data = NULL!");
        return;
    }

    static_assert(std::numeric_limits<float>::is_iec559, "Only support IEC 559 (IEEE 754) float");
    memset(data, 0, datasize * sizeof(float));
}

/** multiplication, will be vectorized */
void FloatWave::multiplyBy(double v,int channel)
{
    float f = v;
    if (channel<0) {
        int n = datasize;
        float* p = data;
        while (n-->0) {
            *p++ *= f;
        }
    } else {
        int n  = sampleCount;
        float* p = data + channel;
        while (n-->0) {
            *p *= f;
            p+=channels;
        }
    }
}

void FloatWave::calcAverage1(FloatWave & out)
{
    float f = 1.0F / sampleCount;
    out.create(samplingRate*f,channels,1);
    float * q = out.getPtr();
    for (int channel=0;channel<channels;channel++) {
        float sum = 0.0;
        float* p = data+channel;
        for (int sub=0;sub<sampleCount;sub++) {
            sum+=*p++;
        }
        *q++ = sum*f;
    }
}

int FloatWave::getNonzeroSampleCount(int channel)
{
    if (data==0) {
        return 0;
    }
    if (channel<0) {
        int r = 0;
        for (int c=0;c<channels;c++) {
            float* p = data+c;
            int n = sampleCount;
            while (n-->0) {
                if (*p++ != 0.0F) r++;
            }
        }
        return r;
    } else {
        int n = sampleCount;
        int r = 0;
        float* p = data+channel;
        while (n-->0) {
            if (*p != 0.0F)  r++;
            p+=channels;
        }
        return r;
    }
}

void FloatWave::create(double f, int ch, int s, bool dirty)
{
    // optimization, avoid mallocs
    long newdatasize = s*ch;
    if (newdatasize<=datasize && data!=0) {
        samplingRate = f;
        channels     = ch;
        sampleCount  = s;
        if (!dirty) {
            zero_inline();
        }
        return;
    }
    if (data!=0)  {
//        LogFmtA("M_ FloatWave free %d",datasize);
        free(data);
        data=0;
        datasize=0;
    }
    samplingRate = f;
    channels     = ch;
    sampleCount  = s;
    if (ch>0 && s>0) {
        datasize=ch*s;
#if defined(__STDC_IEC_559__)
        // float is defined to be zero with all bits zero
        // so let the runtime do its zero-page magic.
        // This also means that the page is not allocated
        // i.e. memory is pinned when first writing an
        // non-zero value with probably good affinity.
        // If you need to pin memory, call zero explicitly.
        data = (float*)calloc(datasize,sizeof(float));
#else
        data = (float*)malloc(datasize*sizeof(float));
        if (!dirty) {
            zero_inline();
        }
#endif
        if (data==0) {
            throw std::bad_alloc();
        }
//        LogFmtA("M_ FloatWave alloc %d",datasize);
    }
}

void FloatWave::setSampleCount(long n)
{
    // optimization, avoid mallocs
    long newdatasize =n*channels;
    if (newdatasize>datasize) {
        data = (float*)realloc(data,newdatasize*sizeof(float));
        if (data==0) {
            throw std::bad_alloc();
        }
        datasize = newdatasize;
//        LogFmtA("M_ FloatWave realloc %d",datasize);
    }
    sampleCount=n;
}

void FloatWave::setGeometry(long n,int c)
{
    // optimization, avoid mallocs
    long newdatasize =n*c;
    if (newdatasize>datasize) {
        data = (float*)realloc(data,newdatasize*sizeof(float));
        if (data==0) {
            throw std::bad_alloc();
        }
        datasize = newdatasize;/*
        LogFmtA("M_ FloatWave realloc %d",datasize);*/
    }
    sampleCount=n;
    channels=c;
}

void FloatWave::setToFloat(FloatWave & other)
{
    setGeometry(other.sampleCount,other.channels);
    setSamples(other.getPtr(),datasize);
    samplingRate = other.samplingRate;
}

void FloatWave::setTo(Wave & other)
{
    setGeometry(other.getFrameCount(),other.getChannels());
    samplingRate = other.getSamplingFrequency();
    for (int i=0;i<sampleCount;i++) {
        for (int c=0;c<channels;c++) {
            setSample(i,c,other.getSample(i,c));
        }
    }

}

void FloatWave::setSamples(float* newData,int number,int startIndex)
{
    if (startIndex+number>sampleCount) {
        throw std::runtime_error("FloatWave setSamples size mismatch.");
    }
    memcpy(data+startIndex, newData, number*sizeof(float));
}

double FloatWave::getSample(long p,int chan) const
{
    if (data==0) {
        qWarning("FloatWave.getSample %d %d no data buffer!",p,chan);
        throw WaveIndexOutofRangeException("FloatWave.getSample");
        return 0.0;
    }
    if (p>=sampleCount ||  chan>=channels  || p<0 || chan<0) {        
        qWarning("FloatWave.getSample %d %d out of range!",p,chan);
        throw WaveIndexOutofRangeException("FloatWave.getSample");
        return 0.0;
    }
    return data[p*channels+chan];
}

void FloatWave::setSample(long p,int chan,double v)
{
    if (data==0) {
        qWarning("FloatWave.setSample %d %d = %f no data buffer!",p,chan,v);
        throw WaveIndexOutofRangeException("FloatWave.setSample");
    }
    if (p>=sampleCount ||  chan>=channels  || p<0 || chan<0) {
        qWarning("FloatWave.setSample %d %d out of range!",p,chan);
        throw WaveIndexOutofRangeException("FloatWave.setSample");
    }
    data[p*channels+chan]=v;
}

int FloatWave::getNextNonzeroIndex(long p,int chan, int & hint, double &v) const
{
    int n = sampleCount;
    p++;
    float * d = data+chan+p;
    while (p<n) {
        assert(d-data < channels*sampleCount);
        float w = *d;
        d++; p++;
        if (fabs(w)>0.0F) {
            v=w;
            hint=p;
            return p;
        }
    }
    return hint=-1;
}

double FloatWave::getMaxSample(int ch) const
{
    if (ch<0) {
        int n  = getDataSamplesTotal();
        float* p = data;
        float ma = 0;
        while (n-->0) {
            float d =fabs(*p++);
            if (d>ma) ma=d;
        }
        return ma;
    }

    int n  = getDataSamplesTotal()/channels;
    float* p = data+ch;
    float ma = 0;
    while (n-->0) {
        float d =fabs(*p);
        if (d>ma) ma=d;
        p+=channels;
    }
    return ma;
}

/** moves data left when samples is positive. */
void FloatWave::shiftBy(int samples,bool dirty)
{
    int o = samples*channels;
    if (o>0) {
        // move left
        memmove(data,data+o,(datasize-o)*sizeof(float));
        if (!dirty) {
            static_assert(std::numeric_limits<float>::is_iec559, "Only support IEC 559 (IEEE 754) float");
            memset(data+datasize-o,0,o*sizeof(float));
        }
    } else {
        memmove(data-o,data,datasize+o);
        if (!dirty) {
            static_assert(std::numeric_limits<float>::is_iec559, "Only support IEC 559 (IEEE 754) float");
            memset(data,0,-o*sizeof(float));
        }
    }
}
