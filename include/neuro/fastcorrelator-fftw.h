#pragma once
#ifdef USE_FFTW
#include "audio/AP_SigProc.h"
#include "audio/AP_wav.h"
// #include <complex.h> 
#include <fftw3.h>
#include "APTL/aplogfmt.h"
#include <stdexcept>
#include "neuro/correlator.h"

#ifdef  WIN32
typedef unsigned __int64 uint64_t ;
#endif

namespace Neuro
{

class FastWavCorrelatorFFTW: public WavCorrelator
{

protected:
	double* window;
	int nu;
	int windowLength;
    fftw_plan p1,p3;		
	bool planned;
    int windowType;


    /** setup window and fftw 
     * @param winlen if given, nu is defined by window size
     */
    void plan(int winlen=0)
    {
#pragma omp critical 
        {                        
            if (winlen>0) {
                nu = ceil( log((double)winlen) / log(2.0));
            }            
            int len = (1<<nu);
            
            fftw_complex *c1,*c2,*c3;
            c1 = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * len);
            c2 = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * len);
            c3 = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * len);		
            p1 = fftw_plan_dft_1d(len, c1, c1, FFTW_FORWARD, FFTW_MEASURE);        
            p3 = fftw_plan_dft_1d(len, c3, c3, FFTW_BACKWARD, FFTW_MEASURE);		         
            fftw_free(c1); 
            fftw_free(c2); 
            fftw_free(c3); 
            
            window = (double*)malloc(len*sizeof(double));
            for (int i=0;i<len;i++) {
                window[i]=0.0;
            }
            if (winlen<=0) {
                windowLength=len;
                
            } 
            int off = (len-winlen)/2;                
            make_window(windowType,window+off,winlen);
            windowLength=winlen;                    
             
            planned=true;
        }                
    }
    
    void unplan()
    {
        if (!planned) return;        
#pragma omp critical 
        {        
            fftw_destroy_plan(p1); 
            fftw_destroy_plan(p3);        
            if (window!=0) {
                free(window);
            }        
            window=0;
            planned=false;
        }
    }
    
public:   
	
    FastWavCorrelatorFFTW(int l=0,int t=0) : window(0), nu(0), windowLength(0), planned(false), windowType(1)
    {
		if (l>0) {
			setWindow(l,t);
		}
    }

	virtual ~FastWavCorrelatorFFTW()
	{		
        unplan();
	} 
    
    void setNu(int nn)
    {
        if (nu==nn) return;
        unplan();
        nu=nn;         
        plan();
    }
    
    /**
     * define Window used
     * @param len  length in samples
     * @param type window type
     */
    void setWindow(int len, int type=-1)
    {
        if (windowLength==len) return;
        unplan();                
        plan(len);
        windowLength=len;
        if (type>0) {
            windowType=type;
        }
    }
    
   virtual bool calcForChannels(int channels,Audio::Wave&in1,int channel1,Audio::Wave&in2,int channel2,Audio::Wave&out,int step=-1)
    {
		if (nu<1 || window==0) {
			throw new std::runtime_error("FastWavCorrelator not initialized");
			return false;
		}
		int len = (1<<nu);
		if (step<=0) {
			step = 1<<(nu-1);
		}
        // LogFmtA("FastWavCorrelator.calc %D FFTWs for %D channels over %D samples, step = %D / %.1fms, window = %D / %.1fms",len,channels, in1.GetSampleCount(), step, (step*1e3 )/ in1.getSamplingFrequency(), windowLength, (windowLength*1e3)/in1.getSamplingFrequency() );		
        
        long samplesIn = in1.getSampleCount();
        int samplesOut = (((uint64_t)(samplesIn-len)) / (uint64_t)step);
		double tout = ((double)step*(double)samplesOut)/(in1.getSamplingFrequency());
        double fout =  (samplesOut / tout);        		
        out.create(fout, channels, samplesOut );
		out.zero();
		Audio::AP_wav rev;
		rev.Create(in1.getSamplingFrequency(), in1.getSampleCount()*2, 16, 1);
		in2.copyChannelReverse(&rev, channel2);
		fftw_complex *c1,*c2,*c3;
		c1 = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * len);
		c2 = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * len);
		c3 = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * len);		
		int mid = channels/2;
		static const double f2 = 1.0 / (double)len;
		long sampleCount = in1.getSampleCount(); 
		long pos1 = 0;
		long pos2 = sampleCount-len-1;
		static const double emptyThreshold = (double)len / 100.0;
        for (int index=0;index<samplesOut;index++) {
            if (pos2<0) {
                LogMsgA("E_ Sample count off!");
                break;            
            }
			double s1=0,s2=0;
			for (int i=0;i<len;i++) {		
				double v1 = in1.getSample(pos1+i,channel1)*window[i];
				s1+=fabs(v1);
				double v2 = rev.getSample(pos2+i,0)*window[i];
				s2+=fabs(v2);
				c1[i][0] = v1;                
				c2[i][0] = v2;
				c1[i][1] = c2[i][1] = 0.0;
			}		
			if (s1 + s2  > emptyThreshold) { // do not correlate zeroes			
				fftw_execute_dft(p1,c1,c1);
				fftw_execute_dft(p1,c2,c2);
				for (int i=0;i<len;i++) {		
					c3[i][0] = c1[i][0]*c2[i][0] - c1[i][1]*c2[i][1];
					c3[i][1] = c1[i][1]*c2[i][0] + c1[i][0]*c2[i][1];				
				//	c3[i][0] = c1[i][0]*c1[i][0] - c1[i][1]*c1[i][1];
				//	c3[i][1] = c1[i][1]*c1[i][0]*2.0;
				}
				fftw_execute_dft(p3,c3,c3);
				for (int channel=0;channel<mid;channel++) {				
					int idx = channel;
                    out.setSample(index, channel+mid, f2*sqrt(c3[idx][0]*c3[idx][0]+c3[idx][1]*c3[idx][1]));
					idx = len-mid+channel;
                    out.setSample(index, channel, f2*sqrt(c3[idx][0]*c3[idx][0]+c3[idx][1]*c3[idx][1]));
				}             
			}
			pos1+=step;
			pos2-=step;
            
        }
//		in1.CopyCuesTo(&out);
		
		fftw_free(c1); fftw_free(c2); fftw_free(c3); 
		
        return true;
    }

};


}

#endif
