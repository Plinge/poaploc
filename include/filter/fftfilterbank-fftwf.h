#pragma once
#include "filter/fftfilterbank.h"
#include <fftw3.h>

/**
 * @ingroup audio
 * @class FftFilterbankFFTW
 * @brief A Filerbank realized using overlap-add and FFTW
 * @author Axel Plinge
 *
 * Note that plan and unplan have to be exclusive and can not be multithreaded
 *
 * @par history
 *   - 2010-02-28  axel   extracted
 *   - 2012-11-03  axel   float 
 */
class FftFilterbankFFTWF : public FftFilterbank<float>
{
protected:
    fftwf_plan plan_dft, plan_idft;

    float normalizeFactor;
    volatile bool planned;

    //! make FFTW plan
    inline void plan();
    //! discard FFTW plan
    inline void unplan();

    QVector< QVector<float> > overhang;


public:
    FftFilterbankFFTWF(int bandCount=25, double fmin=150.0, double fmax=9000.0, double samplingFrequency=96000.0, Filtering::Spacing s = Filtering::LINEAR, Filtering::FilterShape m = Filtering::RECTANGULAR, double scale=1.0);

    virtual ~FftFilterbankFFTWF();
    /** non virtual init */
    void initBuffers();
    /** virtual init */
    virtual void init();
    /** non virtual cleanup */
    void clearBuffers();
    /** virtual cleanup */
    virtual void clear();
    /** change order */
    virtual void setNu(int nn);

    /** calculate */
    virtual float* calcBuffer(unsigned int length,float* in,float *out,int positionInfo = Filtering::HAVE_BOTH) {
        return calcBufferSlice(length,in,out,0,getCount(),positionInfo);
    }

    virtual float * calcBufferSlice(unsigned int sampleCount,float* in,float* out,int bandStart,int bandEnd,int positionInfo);

};
