#pragma once
#include "neuro/spikegenerator.h"
#include "audio/AP_wav.h"
#include "audio/Filter/MovingAverage.h"
#include "audio/AP_SigProc.h"

namespace Neuro {

/** peak-over-average spikes */
class SpikeGeneratorPOA : public  SpikeGenerator
{
protected:
	double averageTime;
	double spikeWidth;
	double energyThreshold;
	double energyFactor;
	double energyExponent;
	double relativeThreshold;
	int mode;
    bool precedenceMode;
public:
	SpikeGeneratorPOA() 
		: averageTime(28.0), 
		energyThreshold(0.05),  energyFactor(0.05), energyExponent(0.5), 
		spikeWidth(0.05), mode(0), relativeThreshold(-9999.0), precedenceMode(false)
	{
	}

	virtual void setMode(int i,bool pd=false) 
	{
		mode=i;
		precedenceMode=pd;
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

	virtual void setSpikeWidth(double us) {
		spikeWidth=us*1e-3;
	}	
    
    virtual bool spikify(Audio::AP_wav&in, Audio::AP_wav&out, ProgressIndicator* progress = 0)
    {		
		out.Create(&in);
		out.SetZero();
        if (progress) {
            progress->setMax(in.GetSampleCount()*2);
        }
        int progressCycle=0;
		int spikeSamples = spikeWidth*1e-3*out.GetFreq();
		if (spikeSamples<1) {
			LogMsgA("W_ Spikes to short for sampling frequnecy!");
			spikeSamples=1;
		}
        Audio::AP_wav mean;
		mean.Create(&in);
#pragma omp parallel for
        for (int channelIndex=0;channelIndex<in.GetNChan();channelIndex++) {
            MovingAverage avg;
            if (mode == 8) {
                avg.SetLength(in.GetFreq() * 1e-2);
            } else {
                avg.SetLength(in.GetFreq() * 1e-3 * averageTime);
            }
			int delay = avg.GetLength() >> 1;
            if (precedenceMode) {
                if (mode == 8) {
                    delay=ceil(in.GetFreq() * 1e-3);
                } else {
                    delay=ceil(in.GetFreq() * 3e-3);
                }
            }
			for (int sampleIndex=0;sampleIndex<delay;sampleIndex++) {				
				 mean.SetSampleValue( sampleIndex, 0, channelIndex );
			}
			int sampleIndexIn =0;
            for (int sampleIndex=delay;sampleIndex<in.GetSampleCount();sampleIndex++) {				
                mean.SetSampleValue( sampleIndex, avg.Calc( abs( in.GetSampleValue(sampleIndexIn++, channelIndex) ) ), channelIndex );
                if (channelIndex==0 && progress && progressCycle++ > 512) {
                    progress->setPos(sampleIndex);
                    progressCycle=0;
                }
            }
        }
	    static const double f1 = 1.0 /32768.0;
#pragma omp parallel for
        for (int channelIndex=0;channelIndex<in.GetNChan();channelIndex++) {
            double sum = 0;
			short spike;
			int length  = 0;
            short vold = 0;
			int poaold = 0;
			bool above = false;
			int index1 = 0;
            int indexMax = 0;
            int poaMax = 0;
			double dbPoa;
			int sampleIndexOut = 0;
			switch (mode) {
			case 2:
            case 3:
				for (int sampleIndex=0;sampleIndex<in.GetSampleCount();sampleIndex++) {
					int v = in.GetSampleValue(sampleIndex,channelIndex);
					int poa = v - mean.GetSampleValue( sampleIndex, channelIndex);					
					if (poa < 0) {
						if (poaold >= 0 && sum > energyThreshold) {						
							length = spikeSamples;
							spike = clip15s( sum * energyFactor * 2.0 * 32767.0 );                            
							above=false;
							int index = (sampleIndex+index1-length);
							if (index<0) index=0; 
							index>>=1;
							for (int i=0;i<length;i++) {
								out.SetSampleValue(index+i, spike, channelIndex);
							}
                            if (mode==3) {
                                for (int i=0;i<=(length>>1);i++) {
                                    out.SetSampleValue(index+length+i, -(spike>>1), channelIndex);
                                    if (index>i+1) {
                                        out.SetSampleValue(index-1-i, -(spike>>1), channelIndex);
                                    }
                                }    
                            }
						} 
					} else {
						if (poaold < 0) {						
							sum = 0.0;
							index1=sampleIndex;
							above=true;
						}
						sum += pow(fabs((double)poa)*f1,energyExponent); 				
					}
					poaold = poa;
					vold = v;					         								
					if (channelIndex==0 && progress && progressCycle++ > 512) {
						progress->setPos(sampleIndex + in.GetSampleCount() );
						progressCycle=0;
					}
				}
				break;
                
                    
            case 4:                
                for (int sampleIndex=0;sampleIndex<in.GetSampleCount();sampleIndex++) {
                    int v = in.GetSampleValue(sampleIndex,channelIndex);
                    int poa = v - mean.GetSampleValue( sampleIndex, channelIndex);                
                    if (poa < 0) {
                        if (poaold >= 0 && sum > energyThreshold) {						
                            length = spikeSamples;
                            double l = (sampleIndex-index1);
                            spike = clip15s( pow(sum/l,energyExponent) * l * energyFactor * 2.0 * 32767.0 );    
                            above=false;
                            int index = (sampleIndex+index1-length);
                            if (index<0) index=0; 
                            index>>=1;
                            for (int i=0;i<length;i++) {
                                out.SetSampleValue(index+i, spike, channelIndex);
                            }                             
                        } 
                    } else {
                        if (poaold < 0) {						
                            sum = 0.0;
                            index1=sampleIndex;
                            above=true;
                        }
                        sum += fabs((double)poa)*f1; 				
                    }
                    poaold = poa; 
                    vold = v;                                                                    
                    if (channelIndex==0 && progress && progressCycle++ > 512) {
                        progress->setPos(sampleIndex + in.GetSampleCount() );
                        progressCycle=0;
                    }
                }
                break;
                    
            case 5:
            case 7:
            case 8:
                for (int sampleIndex=0;sampleIndex<in.GetSampleCount();sampleIndex++) {
                    int v = in.GetSampleValue(sampleIndex,channelIndex);
					int a =  mean.GetSampleValue( sampleIndex, channelIndex);
                    int poa = v - a;
					double dbPoa =  a > 0 ? 20.0*log10( (double)v / (double)a) : 0.0;
                    if (poa <= 0) {
                        if (poaold >= 0 && sum > energyThreshold && poaMax > 0) {						
                            length = spikeSamples;
                            spike = clip15s( sum * energyFactor * 2.0 * 32767.0 );                            
                            above=false;                         
                            for (int i=0;i<length;i++) {
                                out.SetSampleValue(indexMax+i, spike, channelIndex);
                            }                         
                            poaMax=0;
                        } 
                    } else {
                        if (poaold <= 0) {						
                            sum = 0.0;
                            indexMax = sampleIndex;
							if (dbPoa>relativeThreshold) {
								poaMax = poa;							
							}
                            above=true;
                        } else {                    
                            if (poa>poaMax) {
                                indexMax = sampleIndex;
								if ( dbPoa>relativeThreshold) {
									poaMax = poa;
								}
                            }
                        }                    
                        sum += pow(fabs((double)poa)*f1,energyExponent); 				
                    }
                    poaold = poa;
                    vold = v;					         								
                    if (channelIndex==0 && progress && progressCycle++ > 512) {
                        progress->setPos(sampleIndex + in.GetSampleCount() );
                        progressCycle=0;
                    }
                }
                break;
            
            case 6:
                for (int sampleIndex=0;sampleIndex<in.GetSampleCount();sampleIndex++) {
                    int v = in.GetSampleValue(sampleIndex,channelIndex);
                    int poa = v - mean.GetSampleValue( sampleIndex, channelIndex);
                
                    if (poa < 0) {
                        if (poaold >= 0 && sum > energyThreshold && poaMax > 0) {						
                            length = spikeSamples;
                            spike = clip15s( sqrt(sum) * 0.25 * energyFactor * 32767.0 );                            
                            above=false;                         
                            for (int i=0;i<length;i++) {
                                out.SetSampleValue(indexMax+i, spike, channelIndex);
                            }                         
                            poaMax=0;
                        } 
                    } else {
                        if (poaold < 0) {						
                            sum = 0.0;
                            indexMax = sampleIndex;
                            poaMax = poa;
                            above=true;
                        } else {                    
                            if (poa>poaMax) {
                                indexMax = sampleIndex;
                                poaMax = poa;
                            }
                        }                    
                        sum += pow((double)poa,2.0)*f1;
                    }
                    poaold = poa;
                    vold = v;					         								
                    if (channelIndex==0 && progress && progressCycle++ > 512) {
                        progress->setPos(sampleIndex + in.GetSampleCount() );
                        progressCycle=0;
                    }
                }
                break;
                
            default:
                for (int sampleIndex=0;sampleIndex<in.GetSampleCount();sampleIndex++) {
					int v = in.GetSampleValue(sampleIndex,channelIndex);
					int poa = v - mean.GetSampleValue( sampleIndex, channelIndex);
					if (v > 0) {
						if (vold<=0) {
							sum = 0.0;
						}
						sum += pow(fabs((double)v)/32768.0,energyExponent); 				
					}                    
					if (poa < 0) {
						if (poaold >= 0 && sum > energyThreshold) {						
							length = spikeSamples;
							spike = clip15s( sum * energyFactor * 32767.0 );                            
						} 
					}
					poaold = poa; 
					vold = v;                					
					if (length>0) {						
						out.SetSampleValue(sampleIndex, spike, channelIndex);
						length--;
					} 	                								
					if (channelIndex==0 && progress && progressCycle++ > 512) {
						progress->setPos(sampleIndex + in.GetSampleCount() );
						progressCycle=0;
					}
				}
				
			
			}
        }
        in.CopyCuesTo(&out);
        return true;
    }
};

}
