#ifndef __Filter__H__039110341384__INCLUDED__
#define __Filter__H__039110341384__INCLUDED__
#pragma once
/**
 * @file
 * @author Axel Plinge
 * @par history
 *  -  06.09.2006  axel@plinge.de  created
 */
#include "filter/clip.h"

/**
 * @ingroup audio
 * @class Filter
 * @brief
 *
 *
 * @author Axel Plinge
 *
 * @par history
 *  -  06.09.2006  axel@plinge.de  created
 *  -  22.07.2009  axel@plinge.de  updated
 */
class Filter
{
protected: // data attributes

public:
    Filter() { }
    virtual ~Filter() { }
    /** filter function. */
    virtual double Calc(double x) = 0;
    /** filter 16bit signed value */
    virtual short Calc16(short x)  {
        return clip15s( Calc((double)x) );
    }
    /** get time delay, exact only for linear phase filters. */
    virtual double GetDelay() { return 0.0; }
    /** reset */
    virtual void Reset() { }
public:
}; // class Filter

/**
 * @ingroup audio
 * @class NullFilter
 * @brief filter returnig 0
 *
 *
 * @author Axel Plinge
 *
 * @par history
 *  -  26.02.2012  ap  created
 */
class NullFilter : public Filter
{
public:
	NullFilter()
	{}

	virtual ~NullFilter()
	{}

    /** filter function. */
    virtual double Calc(double x)
	{
		return 0.0;
	}
    /** filter 16bit signed value */
    virtual short Calc16(short x)  {
        return 0;
    }
}; 

/**
 * @ingroup audio
 * @class AllpassFilter
 * @brief filter returnig input
 *
 *
 * @author Axel Plinge
 *
 * @par history
 *  -  26.02.2012  ap  created
 */
class AllpassFilter : public Filter
{
public:
	AllpassFilter()
	{}

	virtual ~AllpassFilter()
	{}

    /** filter function. */
    virtual double Calc(double x)
	{
		return x;
	}
    /** filter 16bit signed value */
    virtual short Calc16(short x)  {
        return x;
    }
}; 
#endif // __Filter__H__039110341384__INCLUDED__
