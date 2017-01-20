#pragma once
#include "neuro/spikegenerator.h"
#include "wave/wave.h"
#include "wave/sparsewave.h"
#include "wave/floatwave.h"
#include "filter/movingaverage.h"
#include "neuro/histogram.h"
#include <QSharedPointer>

namespace Neuro {

/**
 * peak-over-average spikes.
 */
class SpikeGeneratorPoaPrecedence : public  SpikeGenerator
{
protected:
	double averageTime;
	double energyThreshold;
	double energyFactor;
	double energyExponent;
	double relativeThreshold;
	double precedence;
    bool half;
	//
	double sum;
	double poaold;
	int indexMax;
	double poaMax;

    QSharedPointer<Histogram> histogram;

    void copy(const SpikeGeneratorPoaPrecedence& other)
	{
		averageTime=other.averageTime;
		energyThreshold=other.energyThreshold;
		energyFactor=other.energyFactor;
		energyExponent=other.energyExponent;
		relativeThreshold=other.relativeThreshold;
		precedence=other.precedence;
        half=other.half;		
	}



public:
    SpikeGeneratorPoaPrecedence(double avg=28.0,double pr=3.0,double th=0.05,double rth=0.0)
		: averageTime(avg), energyThreshold(th),  energyFactor(0.05), energyExponent(0.5), 
          relativeThreshold(rth), precedence(pr), half(false), histogram(0)
	{
		reset();
        //log();
	}
	
    SpikeGeneratorPoaPrecedence(const SpikeGeneratorPoaPrecedence& other) : histogram(0)
	{
		copy(other);
        reset();
        //log();
	}

    SpikeGeneratorPoaPrecedence(const SpikeGeneratorPoaPrecedence* other) : histogram(0)
    {
        copy(*other);
        reset();
        //log();
    }

	const SpikeGeneratorPoaPrecedence& operator=(const SpikeGeneratorPoaPrecedence& other) 
	{
		copy(other);
        reset();
        //log();
		return *this;
	}

    inline void log()
    {
//        LogFmtA("SpikeGeneratorPoaPrecedence %s avg %f pre %f mth %f ath %f ee %f %f",
//                (half?"half":"full"), averageTime, precedence, relativeThreshold, energyThreshold, energyExponent, energyFactor);
    }

    virtual void attachHistogram(QSharedPointer<Histogram> h)
    {
        histogram=h;
    }

    virtual Histogram* getHistogram()
    {
        return histogram.data();
    }

    virtual void setLike(const SpikeGeneratorPoaPrecedence& other)
    {
        copy(other);        
        reset();
        log();
    }

    virtual void reset()
	{                
		sum = 0;
		poaold = 0;
		indexMax = 0;
        poaMax = 0;
	}

    virtual void setAverageTime(double a)
    {
        averageTime = a;
    }
    
    virtual void setShiftTime(double s)
    {
        precedence = s;
    }
    
	virtual void setRelativeThreshold(double db)
	{
		relativeThreshold = db;
	}

	virtual void setExponent(double e) 
	{
		energyExponent=e;
	}
    
	virtual void setGain(double g) 
	{
		energyFactor = g * 0.05;
	}

    virtual void setHalfway(bool b)
    {
        half = b;
    }     

	virtual void setEnergyThreshold(double e) 
	{
		energyThreshold=e;
	}

	virtual double getAverageTime() 
	{
		return averageTime;
	}
	
	virtual double getPrecedenceTime()
	{
		return precedence;
	}           

    virtual bool spikify(Audio::Wave &in, Audio::Wave &out)
	{
        if (!spikifyWaves(in,out)) {
			return false;
		}
//		in.CopyCuesTo(&out);
		return true;
	}

    inline void update(Audio::Wave &out, int channelIndex, int sampleIndex, double v, double a)
    {

        double av = half ? (v>0.0?v:0) : fabs(v);
        double poa = av - a;
        double dbPoa =  0.0;
        if (a > 0.0 && av > 0.0) {
            dbPoa = 20.0F * log10(av / a);
        }
        if (poa <= 0) {
            if (poaold >= 0 && sum >= energyThreshold && poaMax > 0) {
                double spike = sum * energyFactor * 2.0;
                //if (spike>1.0) spike=1.0;
                out.setSample(indexMax, channelIndex, spike);
                if (histogram) {
                    histogram->update(spike>0 ? 20.0*log10(spike): -360.0);
                }
                poaMax = 0.0;
            }
        } else {
            if (poaold <= 0) {
                // this is an upcross, poa > 0 && poaold <= 0
                // reset everything
                sum = 0.0;
                indexMax = sampleIndex;
                if (dbPoa > relativeThreshold) {
                    poaMax = poa;
                } else {
                    poaMax = 0;
                }
            } else {
                // we are in an upper wave
                // check for peak
                if (poa > poaMax && dbPoa > relativeThreshold) {
                    indexMax = sampleIndex;
                    poaMax = poa;
                }
            }
            sum += pow(fabs(poa), energyExponent);
        }
        poaold = poa;
    }

    virtual bool spikifyChannel(Audio::Wave&in, Audio::Wave&out, int channelIndex)
    {
        Audio::FloatWave mean;
        mean.create(in.getSamplingFrequency(),  1, in.getFrameCount());
        MovingAverage avg;
        avg.SetLength(in.getSamplingFrequency() * 1e-3 * averageTime);
        int delay = ceil(in.getSamplingFrequency() * 1e-3 * precedence );
        int sampleIndexIn = 0;
        if (delay>=0) {
            for (int sampleIndex=0;sampleIndex<delay;sampleIndex++) {
                mean.setSample(sampleIndex, 0, 0.0);
            }
            for (int sampleIndex=delay;sampleIndex<in.getFrameCount();sampleIndex++) {
                mean.setSample(sampleIndex, 0, avg.Calc(fabs(in.getSample(sampleIndexIn++, channelIndex))));
            }
        } else {
            for (sampleIndexIn=0;sampleIndexIn<-delay;sampleIndexIn++) {
                avg.Calc(fabs(in.getSample(sampleIndexIn, channelIndex)));
            }
            int sampleIndex=0;
            for (;sampleIndexIn<in.getFrameCount();sampleIndexIn++,sampleIndex++) {
                mean.setSample(sampleIndex, 0, avg.Calc(fabs(in.getSample(sampleIndexIn, channelIndex))));
            }
            for (;sampleIndex<-in.getFrameCount();sampleIndex++) {
                mean.setSample(sampleIndex, 0, avg.Calc(0.0));
            }
        }
        reset();
        for (int sampleIndex=0;sampleIndex<in.getFrameCount();sampleIndex++) {
               double v = in.getSample(sampleIndex, channelIndex);
               double a = mean.getSample(sampleIndex, 0);
               update(out, channelIndex, sampleIndex, v, a);
        }
		return true;
    }

    virtual bool spikifyWaves(Audio::Wave&in, Audio::Wave&out)
    {		
        out.create(in.getSamplingFrequency(), in.getChannels(), in.getFrameCount());
		out.zero();
        for (int channelIndex=0;channelIndex<in.getChannels();channelIndex++) {
            spikifyChannel(in,out,channelIndex);
        }
        return true;
    }
};

}
