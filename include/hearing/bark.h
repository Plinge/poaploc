#ifndef __BARK_H_INCLUDED__
#define __BARK_H_INCLUDED__
#pragma once

#include "math.h"

/**
* mel, Bark and ERB calculations
* @par Critical band rate z (in bark): z = [26.81 / (1 + 1960 / f )] - 0.53, with f in Hz 
*  taken from H. Traunmüller (1990) "Analytical expressions for the tonotopic sensory scale" J. Acoust. Soc. Am. 88: 97-100.    
*  This agrees with the experimental values [9, 10] to within +/- 0.05 bark within the frequency rage from 200 Hz to 6.7 kHz. 
*  Inverse: f = 1960 / [26.81 / (z + 0.53) - 1]
* @par An alternative is 6.0 * asinh(f/600.0)
*   Hynek's formula (2003-04-11 dpwe@ee.columbia.edu) taken from rasta/audspec.c
*   it converges less strict >10kHz
* @par 
*/
//#ifdef WIN32
//#ifndef asinh
///** asinh() - calcs the inverse hyperbolic sine of x */
//static inline double asinh(double x) {
//	return log(x + sqrt(x*x + 1));
//}
//#endif
//#endif
/** convert Hz to Bark  */
static inline double hz2bark(double f) {
	if (f<=10.0) return 0.0;
	return (26.81 / (1.0 + 1960.0 / f )) - 0.53;
}

/** convert Bark to Hz */
static inline double bark2hz(double z) {
	return 1960 / (26.81 / (z + 0.53) - 1);
}
/** convert Hz to mel  */
static inline double hz2mel(double f) {
	return 100.0 * hz2bark(f);
}
/** convert mel to Hz  */
static inline double mel2hz(double m) {
	return bark2hz(m/100.0);
}

// ===================

static const double ERB_C1 = 24.673;
static const double ERB_C2 = 4.368;
static const double ERB_C3 = 21.366; //C3=(2302.6/(c1 * c2));

/** convert Hz to ERB  */
static double hz2erb(double freq) {
  return (ERB_C3*log10((ERB_C2 * freq/1000.0) + 1.0));
}

/** convert ERB to Hz */
static  double erb2hz(double erb) {
  return 1000.0 * (pow(10.0,(erb/ERB_C3)) - 1.0) / ERB_C2;
}
#endif
