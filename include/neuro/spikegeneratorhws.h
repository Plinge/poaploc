#pragma once
#include "audio/AP_wav.h"
#include "audio/Filter/MovingAverage.h"
#include "audio/AP_SigProc.h"
#include "neuro/spikegenerator.h"

namespace Neuro
{


class SpikeGeneratorHWS : public SpikeGenerator
{
	double exponent;
	double gain;
public:
    SpikeGeneratorHWS() :  exponent(0.5), gain(1.0)
	{
	}

	virtual void setExponent(double d)
	{
		exponent=d;
	}
    
    virtual void setGain(double g) 
    {
        gain = g;
    }

	virtual bool spikify(Audio::AP_wav&in,Audio::AP_wav&out,ProgressIndicator* progress = 0)
    {
		out.Create(&in);
		if (progress) {
            progress->setMax(in.GetSampleCount());
        }
		int progressCycle=0;
		int channelIndex=0;
#pragma omp parallel for
		for (channelIndex=0;channelIndex<in.GetNChan();channelIndex++) {
			for (int sampleIndex=0;sampleIndex<in.GetSampleCount();sampleIndex++) {				
				short v = in.GetSampleValue(sampleIndex,channelIndex);
				if (v<0) {
					v=0;
				} else {
					v = clip15s( 32767.0 * 0.5 * gain *  pow( ((double)v/32768.0), exponent ) );
				}
				out.SetSampleValue( sampleIndex, v ,channelIndex);			
				if (channelIndex==0 && progress && progressCycle++ > 512) {
					progress->setPos(sampleIndex);
					progressCycle=0;
				}
			}
		}
		in.CopyCuesTo(&out);
		return true;
	}
};


}