#pragma once
#include "wave/wavecombination.h"

/**
 *
 * Combination of multiple waves with the Hamacher fuzzy t-norm.
 *
 */
class HamacherCombination  : public WaveCombination
{
protected:
	double gamma;

	inline double hamacherProduct(double a,double b)
	{
        if (gamma<0) {
            return (a+b)*0.5;
        } else if (gamma>=1.0) {
            return a*b;
        } else {
            return (a*b)/((gamma)+(1.0-gamma)*(a+b-(a*b)));
        }
	}

    inline float hamacherProduct(float a,float b)
    {
        if (gamma<0) {
            return (a+b)*0.5F;
        } else if (gamma>=1.0) {
            return a*b;
        } else {
            return (a*b)/((gamma)+(1.0-gamma)*(a+b-(a*b)));
        }
    }

public:
	HamacherCombination(double g=0.3) : gamma(g)
	{

	}

	void setGamma(double g)
	{
        gamma = g;
	}

protected:
 
    bool combineRacing(Audio::Wave& in,Audio::Wave & add,Audio::Wave& out);
    bool combinePlain (Audio::Wave& in,Audio::Wave & add,Audio::Wave& out);


public:
    virtual QSharedPointer<Audio::Wave> combine(Audio::WavGroup &correlates,QSharedPointer< Audio::Wave > temp);
    virtual void prepare(Audio::Wave &correlate);

};
