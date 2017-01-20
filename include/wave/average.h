#ifndef _WAVE_AVERAGE_H_
#define _WAVE_AVERAGE_H_
#pragma once
#include "wave/wave.h"
#include "wave/sparsewave.h"



 void calcAverage(Audio::Wave& in,Audio::Wave & out,double sampleCountAverage,double sampleCountOut=1.0);
 void calcAverageSparse(Audio::Wave& in,Audio::SparseWave & out,double sampleCountAverage,double sampleCountOut=1.0);


#endif
