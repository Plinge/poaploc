#pragma once
#include "audio/AP_wav.h"
#include "audio/Filter/MovingAverage.h"
#include "audio/AP_SigProc.h"


namespace Neuro
{

 
/** zc spikes */
class SpikeGeneratorZC : public  SpikeGenerator
{
protected:
	double spikeWidth;
	double energyThreshold;
	double energyFactor;
	double energyExponent;
    
public:
	SpikeGeneratorZC() :  energyThreshold(0.05),  energyFactor(0.05), energyExponent(0.5), spikeWidth(0.05)
	{
	}

	void setSpikeWidth(double us) {
		spikeWidth=us*1e-3;
	}

    virtual void setGain(double g) 
    {
        energyFactor = g * 0.05;
    }

	virtual void setExponent(double e) 
	{
		energyExponent=e;
	}

    virtual bool spikify(Audio::AP_wav&in,Audio::AP_wav&out,ProgressIndicator* progress = 0)
    {
        
        out.Create(in.getSamplingFrequency(), in.GetSampleCount()*2*in.GetNChan(), 16, in.GetNChan());
        out.Create(&in);
        if (progress) {
            progress->setMax(in.GetSampleCount());
        }
        int progressCycle=0;
		int spikeSamples = spikeWidth*1e-3*out.GetFreq();
        short spike = 0;
#pragma omp parallel for
        for (int channelIndex=0;channelIndex<in.GetNChan();channelIndex++) {
            double sum = 0;
			int length  = 0;
            short vold = 0;
            int sampleIndexOut=0;
            for (int sampleIndex=0;sampleIndex<in.GetSampleCount();sampleIndex++) {
                int v = in.GetSampleValue(sampleIndex,channelIndex);
                
                if (v < 0) {
					if (vold >= 0) {
						length = spikeSamples;
					} 
                    if (length>0 && sum > energyThreshold) {
                        spike = clip15s( sum * energyFactor * 32767.0 );
                        //length--;
                    } else {
                        sum = 0.0;
                    }	
                } else {
                    if (vold<0) {
                        sum = 0.0;
                    }
					sum += pow(v/32768.0,energyExponent);
                    //sum = 12.0;
                }	
                vold = v;
                
                short vout = 0;
                if (length>0) {
                    vout = spike;
                    length--;
                } 	                			
                out.SetSampleValue(sampleIndex, vout, channelIndex);
                                                
                if (channelIndex==0 && progress && progressCycle++ > 512) {
                    progress->setPos(sampleIndex + in.GetSampleCount() );
                    progressCycle=0;
                }
            }
        }
        in.CopyCuesTo(&out);
        return true;
    }
};


}