#pragma once
#include "audio/AP_wav.h"
#include "audio/AP_WavUtil.h"
#include "audio/clip.h"
#include "neuro/correlator.h"


namespace Neuro {

class CorrelatorRow {
protected:
	double* data1;
	double* data2;
	int n,n1;
	int pos;
	int index1,index2;

public:
	CorrelatorRow(int nn) : n(nn)
	{		
		n1 = n-1;
		data1 = (double*) malloc(sizeof(double)*n);
		data2 = (double*) malloc(sizeof(double)*n);
		for (int j=0;j<n;j++) {
			data2[j]=data1[j]=0.0;
		}
		pos = 0;
	}
	
	virtual ~CorrelatorRow()
	{
		free(data1);
		free(data2);
	}
	
	inline void step()
	{
		pos++;
		if (pos>=n) {
			pos=0;
		}
	}
	
	inline void put(double x,double y) 
	{
		step();
		data1[pos]=x;
		data2[pos]=y;
	}

	inline void put2(double x,double y) 
	{
		put(x,y);
		put(x,y);
	}

	inline void rewind()
	{
		index1 = 0+pos;
		index2 = n1+pos;
		while (index2>=n) index2-=n;
		while (index2<0) index2+=n;
		while (index1>=n) index1-=n;
		while (index1<0) index1+=n;
	}

	inline double nextProd()
	{
		if (index1>=n) index1-=n;
		if (index2<0) index2+=n;
		return  data1[index1++] * data2[index2--];		
	}
};

class JeffressCorrelator : public CorrelatorRow
{
public:
    
	double* values;
    
    JeffressCorrelator(int n) : CorrelatorRow(n)
    {
		values = (double*) malloc(sizeof(double)*n);        		
    }


    virtual ~JeffressCorrelator()
    {
		free(values);        
    }

    inline void calc(double l,double r)
    {
        put2(l,r);rewind();
		double* v = values;
        for (int channel=0;channel<n;channel++) {
			*v++ = nextProd();
        }
    }

    inline double getValue(int channel)
    {
        return values[channel];
    }
};



class JeffressWavCorrelator: public WavCorrelator
{
	int channels;
    int len;
    double * window ;
    int windowType;
public: 	
    JeffressWavCorrelator(int n,int t) : len(n), windowType(t)
    {
        window = (double*)malloc(len*sizeof(double));
        for (int i=0;i<len;i++) {
            window[i]=0.0;
        }        
        make_window(windowType,window,len);
    }
    
    virtual ~JeffressWavCorrelator()
    {
        if (window!=0) {
            free(window);
        }
    }
	void setMaxDelay(int n)
	{
		channels=2*n+1;		
		if (n>512) n=512;
	}

	void setChannels(int j)
	{
		channels=j;
	}
	
	int getChannelCount()
	{
		return channels;
	}

	bool calcForChannelsOneWav(Audio::AP_wav&in,int channel1,int channel2,Audio::AP_wav&out)
    {		
        return calcForChannelsWAV(in,channel1,in,channel2,out);
    }

    virtual bool calcForChannels(int channels,Audio::Wave&in1,int channel1,Audio::Wave&in2,int channel2,Audio::Wave&out,int step=-1)
    {
        LogFmtA("JeffresCorrelator.calc %D channels for %D samples",channels, in1.getSampleCount());
        long samplesIn = in1.getSampleCount();
        Audio::AP_wav tmp;
        
        
        tmp.create(in1.getSamplingFrequency(),  channels, samplesIn );
		tmp.zero();
        
        JeffressCorrelator row(channels);		
		
        for (int sampleIndex=0;sampleIndex<in1.getSampleCount();sampleIndex++) {
            row.calc( in1.getSample(sampleIndex,channel1), in2.getSample(sampleIndex,channel2) );
            for (int channel=0;channel<channels;channel++) {
                tmp.setSample(sampleIndex,channel,row.getValue(channel));
            }
             
        }
        if (step<=0) {
            step = len>>1;
        }
        int samplesOut = (((uint64_t)(samplesIn-len)) / (uint64_t)step);
        double tout = ((double)step*(double)samplesOut)/(in1.getSamplingFrequency());
        double fout =  (samplesOut / tout);        		
        out.create(fout,  channels, samplesOut );
        out.zero();
        long sampleCount = in1.getSampleCount(); 
        long pos1 = 0;        
        // double f3 = 1.0/len;        
        for (int index=0;index<samplesOut;index++) {            
            for (int channel=0;channel<channels;channel++) {				
                double s=0.0;
                for (int i=0;i<len;i++) {		
                    double v1 = tmp.getSample(pos1+i,channel)*window[i];
                    s+=fabs(v1);
                    
                }		
                out.setSample(index, channel, s );    
            }
            pos1+=step;                     
        }
        return true;
    }
    
	bool calcForChannelsWAV(Audio::AP_wav&in1,int channel1,Audio::AP_wav&in2,int channel2,Audio::AP_wav&out)
    {
		LogFmtA("JeffresCorrelator.calc %D channels for %D samples",channels, in1.GetSampleCount());
        out.Create(in1.GetFreq(),in1.GetSampleCount()* channels*2,16, channels);
        JeffressCorrelator row(channels);		
		
		static const double f1=1.0/32768.0;
		static const double f2=16384.0;
        for (int sampleIndex=0;sampleIndex<in1.GetSampleCount();sampleIndex++) {
            row.calc( in1.GetSampleValue(sampleIndex,channel1)*f1, in2.GetSampleValue(sampleIndex,channel2)*f1 );
			for (int channel=0;channel<channels;channel++) {
                out.SetSampleValue(sampleIndex,clip15s(row.getValue(channel)*f2),channel);
            }
             
        }
		in1.CopyCuesTo(&out);
        return true;
    }

	bool calc2(Audio::AP_wav&in,Audio::AP_wav&out)
    {		
        return calcForChannelsOneWav(in,0,1,out);
    }
    
     
};

};