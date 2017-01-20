#pragma once

namespace Neuro {
    /**
      * interface class.
      */
	class WavCorrelator 
	{
	public:	
    virtual bool calcForChannels(int channelsOut, Audio::Wave& in1, int channel1, Audio::Wave& in2, int channel2, Audio::Wave& out, int step=-1,int firstCenter=-99999,int samplesOut=0)=0;
	};
}
