#pragma once
#include "wave/wave.h"
#include <QMap>
#include <QVector>
#include <math.h>

namespace  Audio
{
/**
* @ingroup audio
* @brief sparse multichannel audiodata.
*
* @author Axel Plinge
*
* @par history
*  -  14.07.2010  Axel Plinge  created
*/

#define WAVE_TYPE_SPARSEWAVE 0x11

class SparseWave: public Wave
{
protected:     
    typedef QMap<long, double> ChannelData;
    double samplingFrequency;
    int channels;
    int virtualSampleCount;
    int actualSampleCount;
    bool dirty;
    QVector< ChannelData* > data;
    //int prealloc,prealloc_s;
    static const int WAVE_TYPE = WAVE_TYPE_SPARSEWAVE;

    void update() 
    {
        int m = 0; 
        for (int channel=0;channel<channels;channel++) {
            if (data.at(channel)->count()>0) {
                int m2 = data.at(channel)->keys().last();
                if (m2>m) m = m2;
            }
        }
        actualSampleCount = m;
        dirty=false;
    }

	void createFrom(const Wave & w)
	{
		create(w.getSamplingFrequency(),w.getChannels(),w.getFrameCount());
		w.copySamplesTo(this);
	}
public:
    SparseWave(double f=22050.0,int c=0,int s=0)
        : channels(0), actualSampleCount(0), dirty(false)
    {
        create(f,c,s);        
    }

	SparseWave(const Wave& other) {
		createFrom(other);
	}

	SparseWave(const Wave* other) {
		createFrom(*other);
	}

    virtual ~SparseWave()
    {
        foreach (ChannelData* p, data) {
            delete p;
        }
        data.clear();
		channels=0;
    }

    virtual int getTypeNumber() const
    {
        return WAVE_TYPE;
    }

    void setPrealloc(int n,int s=64*1024)
    {
        //prealloc = n;
        //prealloc_s = s;
    }

    virtual double getSamplingFrequency() const 
    {
        return samplingFrequency;
    }
    
    virtual int getChannels() const 
    {
        return channels;
    }

    virtual void setSampleCount(long n)
    {
        virtualSampleCount=n;
    }

    virtual bool isSparse() const
    {
        return true;
    }

    virtual bool isInfinite() const
    {
        return true;
    }

    virtual void create(double freq,int chan,int vs,bool dirty=false);

    virtual int getChannels()
    {
        return channels;
    }

    virtual void zero();

    virtual int getDataSamples(int channel) const;

    virtual int getDataSamplesTotal() const;

    virtual int getFrameCount() const
    {
        return virtualSampleCount;
    }

    virtual int getActualSampleCount() 
    {
        if (dirty) {
            update();    
        }
        return actualSampleCount;
    }

    virtual double getSample(long p,int chan) const;

    virtual double getSampleSum(int chan=-1) const;


    void calcAverage1(Wave & out);

    virtual void setSample(long p,int chan,double v);

    virtual int getNextNonzeroIndex(long p,int chan, int & hint, double & v) const;

    virtual void copySamplesTo(Wave * dst,long maxpos=-1);

    virtual double getMaxSample(int channel=-1) const;

    virtual void multiplyBy(double v,int ch=-1);

    virtual void shiftBy(int samples, bool dirt=false);

};
}
