#pragma once
#include "neuro/spikecorrelator.h"
#include "neuro/fastcorrelator.h"
#include "neuro/jeffresscorrelator.h"
#include "neuro/fastcorrelator-fftw.h"

namespace Neuro {

class CorrelatorFactory
{
public:
	typedef enum { FFT, SPIKE, SPIKE_NEAREST, SPIKE_FIRST, SPIKE_MAX, JEFFRESS } CorMode;

static WavCorrelator* create(int samples,int windowType,CorMode corMode)
{
    if (corMode == JEFFRESS) {
        return new JeffressWavCorrelator( samples, windowType  );
    }
	if (corMode == SPIKE_FIRST) {
		return new SpikeWavCorrelator( samples, windowType, Neuro::SpikeWavCorrelator::First );
	}
	if (corMode == SPIKE_NEAREST) {
		return new SpikeWavCorrelator( samples, windowType, Neuro::SpikeWavCorrelator::Nearest );
	}
	if (corMode == SPIKE_MAX) {
		return new SpikeWavCorrelator( samples, windowType, Neuro::SpikeWavCorrelator::Max );
	}
	if (corMode == SPIKE) {
		return new SpikeWavCorrelator( samples, windowType, Neuro::SpikeWavCorrelator::All );
	}
#ifdef USE_FFTW
	return new Neuro::FastWavCorrelatorFFTW( samples, windowType);
#else
	return new Neuro::FastWavCorrelator( samples, windowType);
#endif
}

static const char* correlationName(CorMode corMode)
{    
    if (corMode == SPIKE_FIRST) {
        return "First";
    }
    if (corMode == SPIKE_NEAREST) {
        return "Nearest";
    }
    if (corMode == SPIKE_MAX) {
        return "Max";
    }
    if (corMode == SPIKE) {
        return "Match";
    }
#ifdef USE_FFTW
    return "FFTW";
#else
    return "FFT";
#endif
}


};

}