#ifndef ERB_SCALE_H
#define ERB_SCALE_H
#pragma once
#include "hearing/scaling.h"

/**
 * Equal Resonance Bandwidth  scaling for auditory filterbanks
 *
 * @author Axel Plinge
 *
 * @ingroup hearing
 *
 * @par history
 * - 2009-09-01  axel  started
 */
namespace Hearing {
class ErbScaling : public Scaling
{
public:
	static const double C1,C2,C3;
public:
	ErbScaling();
    /** convert Hz to ERB  */
    virtual double scale(double freq);
    /** convert ERB to Hz */
    virtual double unscale(double erb);
};


#endif

} // namespace Hearing
