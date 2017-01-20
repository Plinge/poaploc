#pragma once
#include "wave/wave.h"
#include "filter/clip.h"
#include "neuro/correlator.h"
#include <math.h>

namespace Neuro
{
	

class SpikeWavCorrelator : public WavCorrelator
{
public:
    typedef enum { All, First, Nearest, Max } Mode;

protected:
    int channels;
    int mid;
    Mode mode;
    int windowLength;
    double threshold;
    //
    double* window;
    //
    double* re1;
    double* re2;
    double* re3;
    //
    double* val1;
    int* idx1;
    double* val2;
    int* idx2;


public:
    SpikeWavCorrelator(int len=0, int type=1, Mode m=All, double th=0)
        : channels(0),  windowLength(0),
          mode(m), threshold(th),
          window(0), re1(0), re2(0), val1(0), val2(0), idx1(0), idx2(0)
    {
        if (len>0) {
            setWindow(len);
            mode=m;
        }
    }

    virtual ~SpikeWavCorrelator()
    {
        cleanup();
    }

    int getChannelCount()
    {
        return channels;
    }


protected:

    void setup(int wlen=0);


    void cleanup();

    /**
     * define Window used
     * @param len  length in samples
     */
    void setWindow(int wlen)
    {
        if (windowLength==wlen) {
            return;
        }
        setup(wlen);
    }

     inline bool validChannel(int channel) {
        return (channel>0&&channel<channels);
    }

    inline void setRanged(Audio::Wave &out,int index,int channel,double v)
    {
        if (!validChannel(channel)) return;
        out.setSample(index,channel,v + out.getSample(index,channel) );
    }

    inline void setOutSpike(Audio::Wave &out, double *re1, double *re2,  int index, int i1, int i2)
    {
        int channel = i1-i2+mid;
        short v1 = clip15s(re1[i1]*re2[i2]);
        short v2 = clip15s(re1[i1]*re2[i2]*0.66);
        short v3 = clip15s(re1[i1]*re2[i2]*0.33);
        setRanged(out,index,channel+2,v3);
        setRanged(out,index,channel+1,v2);
        setRanged(out,index,channel  ,v1);
        setRanged(out,index,channel-1,v2);
        setRanged(out,index,channel-2,v3);
    }

    void calcNormal(Audio::Wave& in1,int channel1, Audio::Wave& in2, int channel2, Audio::Wave& out, int step, int firstCenter=-99999,int samplesOut=0);
    void calcSpiked1(Audio::Wave& in1,int channel1, Audio::Wave& in2, int channel2, double* re3 , int pos1);
    void calcSpiked(Audio::Wave& in1,int channel1, Audio::Wave& in2, int channel2, Audio::Wave& out, int step,int firstCenter = -99999,int samplesOut=0);

public:
    virtual void calcOneForChannels(int channelsOut, Audio::Wave& in1, int channel1, Audio::Wave& in2, int channel2, double* result, int pos);
    virtual bool calcForChannels(int channelsOut, Audio::Wave& in1, int channel1, Audio::Wave& in2, int channel2, Audio::Wave& out, int step=-1,int firstCenter=-99999,int samplesOut=0);
};


}
