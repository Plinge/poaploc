#pragma once
#include "hearing/erbscaling.h"
#define _USE_MATH_DEFINES
#include "math.h"

namespace Hearing {
/**
 * Gamatone Response functions.
 * 
 * Spectral definiton based on M. Unoki and M. Akagi [4]
 *
 * @author Axel Plinge
 *
 * @ingroup hearing
 *
 * @par history
 * - 2012-03-09  axel  added bandwidth scaling
 * - 2010-04-13  axel  comments
 * - 2009-12-06  axel  started
 *
 * @par references
 * - [1] B. Glasberg und B. Moore
 *   Derivation of auditory ﬁlter shapes from notched-noise data.
 *   Hearing Research, 47(1–2):103–138, August 1990. 
 * - [2] R. Patterson, I. Nimmo-Smith, J. Holdsworth und P. Rice
 *   An efﬁcient auditory ﬁlterbank based on the gammatone functions. 
 *   Tech.Rep. APU Report 2341, MRC, Applied Psychology Unit, Cambridge U.K, 1988.
 * - [3] M. Slaney: An efﬁcient implementation of the Patterson-Holdsworth auditory ﬁlter bank.
 *   Tech.Rep. 35, Apple Computer, Inc., 1993. 
 * - [4] M. Unoki and M. Akagi
 *   A method of signal extraction from noisy signal based on auditory scene analysis
 *   Speech Commun., vol. 27, no. 3, pp. 261–279, 1999.
 *
 */
class Gammatones 
{

public:
	/** gammatone time function [1,3] */
	static double filter(double t, double fc, double b, double a)
	{
		return a*cos(2.0*M_PI*t*fc)*exp(-2.0*M_PI*b*t);
	}

	/** ERB of fc */
	static inline double bandwidthGlasbergMoore(double fc) 
	{
        return  ErbScaling::C1 * ((ErbScaling::C2 * 1e-3* fc)+1.0);
	}

	/** amplitude of a gammatone filter at frequnency f using G&M bandwitdh [1,3] */
    static inline double amplitudeGlasbergMoore(double f,double fc,double scale=1.0)
	{		
        return amplitude(f,fc,bandwidthGlasbergMoore(fc)*scale);
	}
	
	/** amplitude of a gammatone filter at frequnency f using Patteroson bandwitdh [2] */
	static inline double amplitudePatterson(double f,double fc) 
	{		
		return amplitude(f,fc,bandwidthGlasbergMoore(fc) * 1.019);	
	}

	/** amplitude of a gammatone filter at frequnency f [4] */
	static double amplitude(double f,double fc, double b)
	{
		double d = (fc - f);
		double n1 = pow(b,4.0);
		double d1 = pow(b,4.0)-6.0*pow(b,2.0)*pow(d,2.0)+pow(d,4.0);
		double d2 = 4.0*(b*pow(d,3.0) - pow(b,3.0)*d);
		double a = n1*pow( (pow(d1,2.0)+pow(d2,2.0)), -0.5);
		return a;
	}
};


}

//
