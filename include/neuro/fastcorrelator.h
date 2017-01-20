#pragma once
#include "audio/AP_SigProc.h"
#include "audio/AP_wav.h"
#include "audio/fourier.h"
#include "audio/clip.h"
#include "APTL/aplogfmt.h"
#include <stdexcept>
#include "APLib/bitutil.h"


namespace Neuro
{

class FastWavCorrelator
{
	int channels;
	double* window;
	int nu;
    int windowLength,windowType;

    void setup(int wlen=0)
    {
        if (wlen>0) {
            nu = ceil( log((double)wlen) / log(2.0));
        }            
        int len = (1<<nu);
        if (wlen<=0) {
            wlen=len;                
        } 
        LogFmtA("FastWavCorrelator.calc %D FFTs for %D samples",len,wlen);
        window = (double*)malloc(len*sizeof(double));
        for (int i=0;i<len;i++) {
            window[i]=0.0;
        }        
        int off = (len-wlen)/2;                
        make_window(windowType,window+off,wlen);
        static const double f1 = 1.0/32768.0;
        for (int i=0;i<len;i++) {
            if (window[i]<0.0) {
				window[i]=0.0;
			} else {
				window[i] *= f1;
			}
        }
        windowLength=wlen;
    }
    
public:   
	
    FastWavCorrelator() : channels(0), window(0), nu(0), windowLength(0), windowType(1)
    {
    }

	virtual ~ FastWavCorrelator()
	{
		if (window!=0) {
			free(window);
		}
	}

    
	void setMaxDelay(int n)
	{
		channels=2*n;		
		if (n>511) n=511;
		setNu( (int)ceil(log10((double)channels)/log10(2.0))+1 );				
	}
	
    void setNu(int nn)
    {
        if (nu==nn) return;
        nu=nn;
        setup(0);
    }
    
        
    /**
     * define Window used
     * @param len  length in samples
     * @param type window type
     */
    void setWindow(int wlen, int type=-1)
    {                
        if (windowLength==wlen && type==windowType) {
            return;        
        }
        if (type>=0) {
            windowType=type;
        }
        setup(wlen);                        
    }
    
	int getChannelCount()
	{
		return channels;
	}

    bool calcForChannels(int channels,Audio::AP_wav&in1,int channel1,Audio::AP_wav&in2,int channel2,Audio::AP_wav&out,int step=-1)
    {
		if (nu<1 || window==0) {
			throw new std::runtime_error("FastWavCorrelator not initialized");
			return false;
		}
        int len = (1<<nu);
        if (step<=0) {
            step = 1<<(nu-1);
        }
        if (len<windowLength) {
            throw new std::runtime_error("FastWavCorrelator not correctly initialized");
            return false;
        }
        LogFmtA("FastWavCorrelator.calc %D FFTs for %D channels over %D samples, step = %D / %.1fms, window = %D / %.1fms",len,channels, in1.GetSampleCount(), step, (step*1e3 )/ in1.getSamplingFrequency(), windowLength, (windowLength*1e3)/in1.getSamplingFrequency() );		
        
        long samplesIn = in1.GetSampleCount();
        int samplesOut = (((uint64_t)(samplesIn-len)) / (uint64_t)step);
        double tout = ((double)step*(double)samplesOut)/(in1.GetFreq());
        double fout =  (samplesOut / tout);        		
        out.Create(fout, samplesOut * channels*2, 16, channels);
		Audio::AP_wav rev;
		rev.Create(in1.GetFreq(), in1.GetSampleCount()*2, 16, 1);
		in2.copyChannelReverse(&rev, channel2);

		int cx = len*sizeof(double);
        double* re1 =  (double*)malloc(cx); double* im1 =  (double*)malloc(cx);
        double* re2 =  (double*)malloc(cx); double* im2 =  (double*)malloc(cx);				
        double* re3 =  (double*)malloc(cx); double* im3 =  (double*)malloc(cx);			
		
        int mid = channels/2;
        static const double f2 = 16384.0/ (double)len;
        long sampleCount = rev.GetSampleCount();
        long pos1 = 0;
        long pos2 = sampleCount-len-1;          
        for (int index=0;index<samplesOut;index++) {
            if (pos2<0) {
                LogMsgA("E_ Sample count off!");
                break;            
            }
			for (int i=0;i<len;i++) {		
				re1[i] = in1.GetSampleValue(pos1+i,channel1)*window[i];
				re2[i] = rev.GetSampleValue(pos2+i,0)*window[i];
				im1[i] = im2[i] = 0.0;
			}
			Fourier::fft(nu,re1,im1);
			Fourier::fft(nu,re2,im2);
			for (int i=0;i<len;i++) {		
				re3[i] = re1[i]*re2[i] - im1[i]*im2[i];
				im3[i] = -(im1[i]*re2[i] + im2[i]*re1[i]);
			}
			Fourier::fft(nu,re3,im3);			
			for (int channel=0;channel<mid;channel++) {				
				int idx = channel;
                out.SetSampleValue(index,clip15s( sqrt( re3[idx]*re3[idx] + im3[idx]*im3[idx])*f2 ), channel+mid);
				idx = len-mid+channel;
				out.SetSampleValue(index,clip15s( sqrt( re3[idx]*re3[idx] + im3[idx]*im3[idx])*f2 ), channel);
            }             
			pos1+=step;
			pos2-=step;
        }
		in1.CopyCuesTo(&out);
		free(im1);free(im2);free(im3);
		free(re1);free(re2);free(re3);
        return true;
    }

};


}