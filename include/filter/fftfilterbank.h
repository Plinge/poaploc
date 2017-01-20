#pragma once
#include "filter/filterbankcentered.h"
#include "hearing/gammatone.h"
//#include "audio/AP_SigProc.h"
//#include "audio/fourier.h"


/**
 * @ingroup audio
 * @class FftFilterbank
 * @brief A Filerbank relizes using overla-add
 * @author Axel Plinge
 * @par history
 *   - 2009-12-08  axel   created
 *   - 2009-12-09  axel   generalized
 */
template <class VALUETYPE>
class FftFilterbank : public FilterbankCentered<VALUETYPE>
{
protected: // data
    double samplingFrequency;
    double minFrequency;
    double maxFrequency;	
    double scaling;
    int count;
    int nu;
    int length,length1,length2;
    Filtering::FilterShape filterShape;
    Filtering::Spacing spacing;
    QVector< QVector < float > > coeffs;
    QVector < float > window;

public:
    FftFilterbank(int n=25, double f1=150.0, double fn=6000.0, double fs=22050.0, Filtering::Spacing s = Filtering::LINEAR, Filtering::FilterShape m = Filtering::RECTANGULAR, double scale=1.0);
	virtual ~FftFilterbank();
    virtual void setup(int n, double f1, double fn, double fs=-1, Filtering::Spacing s=Filtering::UNDEFINED, Filtering::FilterShape m=Filtering::NOSPLIT, double scale=1.0);
    /**  change order */
    virtual void setNu(int nn);
    /** virtual initialization */
    virtual void init();
    /** non-virtual initialization */
    void initFilters();
    /** virtual cleanup */
	virtual void clear();
    /** non-virtual cleanup */
    void clearFilters();
    /** @return number of bands */
    virtual int getCount();
    /** set fs */
    virtual void setSamplingFrequency(double fs);
    /** get fs */
    virtual double getSamplingFrequency();
    /** samples overlap, returns negative value to indicate full overlap requirement */
    virtual int getOverlap();
    /** rewind */
    virtual void reset();
    /** set a single coeff */
    inline void setCoeff(int band, int tap, double value) {
        QVector<float> & coeff = coeffs[band];
        coeff[tap]=value;
        coeff[length1-tap]=value;
    }
    /** get all coeffs */
    inline const QVector<float> & getCoeffs(int bandIndex) {
        return coeffs[bandIndex];
    }
    


    /** get gain of band */
    virtual double getBandGain(int index);
    
    /** set gain of band */
    virtual void setBandGain(int index,double gain);
    
    /** change gain of band */
    virtual void addBandGain(int index,double gain);
	    
    /** calculate sliced, somewhat inefficient for band split since fft of input singal is repeated */
    virtual VALUETYPE* calcBufferSlice(unsigned int length,VALUETYPE* in,VALUETYPE* out,int bandStart,int bandEnd,int positionInfo=0)=0;
        
    static void sine_window(int length, float* window)
    {
        float fact = 2.0F * M_PI / (float)length;
        for (int i = 0; i < length; i++) {
            window[i] = 0.5F-0.5F*cos( fact*(float)i );
        }
    }

};

// implementation
#include "filter/fftfilterbank_impl.h"
//
