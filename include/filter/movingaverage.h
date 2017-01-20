#ifndef __MovingAverage__H__039110341384__INCLUDED__
#define __MovingAverage__H__039110341384__INCLUDED__
#pragma once
/**
 * @file
 * @author Axel Plinge
 * @par history
 *  -  02.10.2007  axel@plinge.de  created
 */
#include "filter/delayfilter.h"

/**
 * @ingroup audio
 * @class MovingAverage 
 * @brief Moving Average of the last length samples.
 * @author Axel Plinge
 * @par history
 *   - 16.07.2005 axel  created
 */
class MovingAverage : public DelayFilter
{
protected:
	double sum;	  /**< sum div len */
	double len_1; /**< one div len */

public:
	MovingAverage() : sum(0.0), len_1(1.0)
	{
	}

	MovingAverage(int l) : sum(0.0), len_1(1.0)
    {
        SetLength(l);
    }
    
	virtual void SetLength(int l)
	{
		if (l<=0)
			len_1=1.0;
		else
			len_1=1.0/(double)l;
		DelayFilter::SetLength(l);
	}


	virtual double Calc(double v)
	{
        if (iLength<=0) return v;
		double vold = DelayFilter::Calc(v);		
		sum += (v-vold)*len_1;
		return sum;
	}

	virtual short Calc16(short v)
	{
		return (short) floor(32767.0 * Calc((double)v / 32768.0) +0.5 );
	}

	virtual void Reset()
	{
		DelayFilter::Reset();
		sum=0.0;
	}


	virtual double GetDelay() { 
		return GetLength()/2; 
	}
};

#endif // __MovingAverage__H__039110341384__INCLUDED__
