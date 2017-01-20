#pragma once
/**
 * @file
 * @author Axel Plinge
 * @par history
 *  -  02.10.2007  Axel Plinge  created
 */

#include "filter/filter.h"

/**
 * @ingroup audio
 * @class DelayFilter 
 * @brief A simple delay with ring buffer.
 * @author Axel Plinge
 * @par history
 *   - 22.06.2005 Axel Plinge commented
 */
class DelayFilter : public Filter
{
public:
	DelayFilter();
    DelayFilter(int l);
	virtual ~DelayFilter();
public:
	virtual void SetLength(int l);
	virtual int GetLength();
	virtual double Calc(double v);	
	virtual void Reset();
	virtual double GetDelay();
    virtual double GetHistory(int index);
protected:		  
	int iLength;	/**<  filter length in samples. */
	double* memory; /**< filter memory. */
	int mempos;     /**< position in memory. */

	double getAt(int index)
	{
        if (memory==0) return 0.0;
		return memory[index];
	}
};
