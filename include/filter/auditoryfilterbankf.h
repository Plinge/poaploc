#pragma once

#include "hearing/ear.h"

#ifdef USE_FFTW
#include "filter/fftfilterbank-fftwf.h"
#else
#error "need fftw"
#endif

/**
 * @ingroup audio
 * @class AuditoryFilterbank
 * @brief A GT ERB Filterbank with outer-middle-ear gain realized as FFT-OA
 * @author Axel Plinge
 * @par history
 *   - 2010-02-15  axel FFTW3
 *   - 2010-02-08  axel created
 *   - 2012-11-06  axel float version
 */
class AuditoryFilterbankF: public FftFilterbankFFTWF
{
   int transFun;

	void setGains()
	{
        if (transFun>0) {
            //LogMsgA("AuditoryFilterbank using outerMiddleTransfer gain");
            for (int band=0;band<getCount();band++) {
                setBandGain(band, 0.25 * Hearing::Ear::outerMiddleTransferTerhardt(getFrequency(band)));
            }
        } else if  (transFun<0) {
            //LogMsgA("AuditoryFilterbank using reverse outerMiddleTransfer gain");
            for (int band=0;band<getCount();band++) {
                setBandGain(band, 0.25 / Hearing::Ear::outerMiddleTransferTerhardt(getFrequency(band)));
            }
        } else {
            //LogMsgA("AuditoryFilterbank using unity gain");            
            for (int band=0;band<getCount();band++) {                
                setBandGain(band, 1.0);
            }           
        }
	}
    

public:
    AuditoryFilterbankF(int bandCount=25, double fmin=150.0, double fmax=9000.0, double samplingFrequency=96000.0,double scaling=1.0)
    : FftFilterbankFFTWF(bandCount, fmin, fmax, samplingFrequency, Filtering::ERB, Filtering::GAMMATONE, scaling), transFun(0)
	{
		setGains();
	}
	
    virtual ~AuditoryFilterbankF()
    {
    }
    
    virtual void setGainMode(int m)
    {
        transFun=m;
        setGains();
    }
    
    void setup(int bandCount,double fmin,double fmax,double samplingFrequency,int tf = 0)
	{        
        FftFilterbankFFTWF::setup(bandCount, fmin, fmax, samplingFrequency, Filtering::ERB, Filtering::GAMMATONE);
        transFun = tf;
		setGains();        
	}	


};
