#ifndef FILTERBANK_H
#define FILTERBANK_H
#pragma once

#include "hearing/bark.h"
#include "filter/filtering.h"
#include <QVector>

namespace Filtering
{

/** spacing of center frequency */
typedef enum SpacingTag {
    UNDEFINED=0,
    LINEAR,
    ERB,
    BARK,
    SII
} Spacing;

/** filter shape */
typedef enum {
    NOSPLIT,
    RECTANGULAR,
    TRIANGULAR,
    GAMMATONE,
    GAUSSIAN,
    CISOID
} FilterShape;

/** number of SII critical bands */
static const int SII_CB_COUNT = 25;

/** band edges of SII critical bands */
static const double SII_CB_LIMITS[SII_CB_COUNT+1] = {100,200,300,400,510,630,770,920,1080,1270,1480,1720,2000,2320,2700,3150,3700,4400,5300,6400,7700,9500};

}

/**
 * a filter bank
 *
 * @author Axel Plinge
 *
 * @par history
 *   - 2009-12-08  axel  extracte
 *   - 2009-12-09  axel  moved spacing here
 *   - 2012-11-05  axel  template
 */
template <class VALUETYPE>
class Filterbank
{
public:    
	
    /** @return number of bands */
	virtual int getCount() = 0;

    /** set fs */
    virtual void setSamplingFrequency(double fs )=0;

    /** get fs */
    virtual double getSamplingFrequency()=0;

    /** samples overlap.
     *  positive values indicate a required number extra input data
     *  negative values indicate a required overlapping of both input and output data, e.g. for overlap-add
     */
    virtual int getOverlap()
    {
            return 0;
    }

    /** rewind */
    virtual void reset() = 0;

	/** calculate */
    virtual VALUETYPE* calcBuffer(unsigned int length,VALUETYPE* in,VALUETYPE *out,int positionInfo = Filtering::HAVE_BOTH) {
        return calcBufferSlice(length,in,out,0,getCount(),positionInfo);
    }

    /** calculate part (for parallelization) */
    virtual VALUETYPE* calcBufferSlice(unsigned int length,VALUETYPE* in,VALUETYPE* out,int bandStart,int bandEnd,int positionInfo=0) = 0;


    /** get frequencies spaced by rule. */
    QVector<double> getCenterFrequencies(int count,Filtering::Spacing spacing,double minFrequency,double maxFrequency)
    {
           QVector<double> fs;
           fs.reserve(count);
            
		   if (count<1) {
               //LogMsgA("E_ Filterbank.getCenterFrequencies No bands?!");
			   return fs;
		   }
           if (count==1) {
               fs.append(0);
               fs.append(minFrequency);
               fs.append(maxFrequency);
               return fs;
           }
           switch (spacing) {
           case Filtering::BARK:
           { 
                double barkMin = hz2bark(minFrequency);
                double barkRange = hz2bark(maxFrequency) - barkMin;
                for (int i=-1;i<=count;i++) {
                    double f = bark2hz( barkMin + (barkRange*i) / (double) (count-1) );
                    fs.append(f);
                }
                break;
           }
           case Filtering::ERB:
           {
               double erbMin = hz2erb(minFrequency);
               double erbRange = hz2erb(maxFrequency) - erbMin;
               for (int i=-1;i<=count;i++) {
                   double f = erb2hz( erbMin + (erbRange*i) / (double) (count-1) );
                   fs.append(f);
               }
           }
           default:
           {
               double hzMin = minFrequency;
               double hzRange = (maxFrequency) - hzMin;
               for (int i=-1;i<=count;i++) {
                   double f = hzMin + (hzRange*i) / (double) (count-1) ;
                   fs.append(f);
               }
           }
           }
        return fs;
    }
};



#endif // FILTERBANK_H
