#pragma once

#include "filter/auditoryfilterbank.h"
#include "audio/neuro/cochleogram.h"
#include "neuro/spikegeneratorpoapd.h"
#include "audio/hearing/gammatone.h"
#include "audio/Filter/wavfilter.h"
#include "StdGUI/progressindicator.h"


namespace Neuro {

class MultiCochlearModelWav
{
protected:
	AP_ArrayPtr<Audio::AP_wav> filtered;
	AP_ArrayPtr<Audio::SparseWave> spiked;
	int channelCount;
	int samplesInput;
	double altGain;
	AuditoryFilterbank filterbank;
	ProgressIndicator* progress;
	bool abortFlag;

	AP_ArrayPtr<Neuro::SpikeGeneratorPoaPrecedence> spikers;

public:

MultiCochlearModelWav() : filterbank(16,200,3600,48000), progress(0)
{
	spikers.AddHead( new Neuro::SpikeGeneratorPoaPrecedence() );
    spikers[0].setGain(4.0);
}

virtual ~MultiCochlearModelWav()
{
	filtered.DeleteAll();
}

virtual double getFrequency(int band) {
	return filterbank.getFrequency(band);
}

virtual void attachProgressIndicator(ProgressIndicator* p)
{
	progress = p; 
}

virtual void setFilter (int bc,double f0, double fn, double a =-1.0) 
{    
	filterbank.setup(bc,f0,fn,48000);
    altGain = a;
}

virtual void setSpikeThres(double t,double e=0.0) 
{
    spikers[0].setRelativeThreshold( t );
    spikers[0].setEnergyThreshold( e );
}
 
 virtual void setSpikePrecedence(double a,double s)
{
    spikers[0].setAverageTime( a );
    spikers[0].setShiftTime( s );
}

virtual	void process (Audio::AP_wav &input) 
{
	filter(input);
	spike();
}

virtual  Audio::Wave & spikes(int index) 
{
	return spiked[index];
}

protected:

virtual	void filter (Audio::AP_wav &input) 
{	
	filtered.DeleteAll();
	abortFlag=false;
	channelCount = input.GetNChan();
	samplesInput =  input.GetSampleCount();
	double samplingFrequency = input.getSamplingFrequency();
	
	if ( samplesInput<=0 ) {
		LogMsgA("W_ MultiCochlearModel.filter NO INPUT!");
		throw new std::runtime_error ( "MultiCochlearModel.filter Error filtering, no input!" );		
	}

 	for ( int mic=0;mic<channelCount;mic++ ) {
		filtered.AddTail ( new Audio::AP_wav );
	}
	// filter
	if ( progress ) {
		progress->setPos ( 0 );
		progress->setMessage ( "Filtering" );
		progress->setMax ( channelCount );
	}
	bool err=false;
	int done=0;

	filterbank.setSamplingFrequency(samplingFrequency);

#pragma omp parallel for
	for ( int mic=0;mic<channelCount;mic++ ) {
		if ( !abortFlag ) {
			try {
				Audio::AP_wav ch;
				ch.Create ( input.GetFreq(),samplesInput*2, 16, 1 );
				input.CopyChannelTo( &ch, mic,-1, 0, 0 );
                
			 
				static const int fadein = 2048;
				static const int fadeskip = 256;
				for (int n=0;n<fadeskip;n++) {
					ch.SetSampleValue(n, 0);
				}
				for (int n=0;n<fadein;n++) {
					ch.SetSampleValue(n+fadeskip, (ch.GetSampleValue(n+fadeskip,0)* n) / fadein);
				}                
                
				if ( !Audio::WavFilter::splitWave ( filterbank,ch,filtered[mic],0,-1,0,999 ) ) {
					err=true;
					abortFlag=true;
				}
			} catch ( std::exception* e ) {
				LogFmtA ( "ERROR while filtering!\n%s", e->what() );
				err=true;
				abortFlag=true;
			}
#pragma omp critical
			{
				if ( progress ) {
					progress->setPos ( ++done );
				}
			}
		}
	}

	if ( err ) {
		throw new std::runtime_error ( "MultiCochlearModel.filter Error while filtering!" );
	}


	double mav=0.0;
	if (altGain<0.0) {
		for ( int mic=0;mic<channelCount;mic++ ) {
			double maa = filtered[mic].getMaxSampleValue();
			mav = std::max<double> ( mav,maa );
		}
		if (mav == 0) mav=1;
		mav = 32767.0 / mav;
		if ( mav>128.0 ) mav = 128.0;
	} else {
		mav = pow(10.0 , altGain/20.0 );
	}
	LogFmtA("MultiCochlearModel.filter Applying gain of %f (set to %f dB)", mav , altGain );
#pragma omp parallel for
	for ( int mic=0;mic<channelCount;mic++ ) {
		filtered[mic].ApplyVolume ( mav );
	}	 
}


virtual void spike() {
	spiked.DeleteAll();
    if (filtered.GetCount()<1)  {
        LogMsgA("E_ MultiCochlearModel.spike: NO FILTERED DATA!");
        throw new std::runtime_error("spike called out of sequence!");
    }
	int  pre = filtered[0].getSampleCount() >> 4;
    for ( int mic=0;mic<channelCount;mic++ ) {
		Audio::SparseWave * s = new Audio::SparseWave();
		s->setPrealloc( pre, 64*1024);
		spiked.AddTail ( s);
    }

    if ( progress ) {
        progress->setPos ( 0 );
        progress->setMessage ( "Spiking" );
        progress->setMax ( channelCount );
    }
    int done=0;
 
    for (int index=1;index<spikers.GetCount();index++) {
        spikers[index].setLike( spikers[0] );
    }
    while (spikers.GetCount()<channelCount) {
        spikers.AddTail( new Neuro::SpikeGeneratorPoaPrecedence( spikers[0] ));
    }

#pragma omp parallel for shared(done)
    for ( int mic=0;mic<channelCount;mic++ ) {
        if ( !abortFlag ) {
			spikers[mic].reset();
            spikers[mic].spikifyWaves( filtered[mic], spiked[mic], 0);
        }
        for ( int filterBand =0;filterBand<spiked[mic].getChannels();filterBand++ ) {        
            double f = filterbank.getFrequency ( filterBand );
            double gain = 1.0;
            if ( f>1000.0 ) {
                gain = ( log ( f/1000.0 ) / log ( 2.0 ) );
            }        
			spiked[mic].multiplyBy  ( pow ( 10.0,gain/20.0 ),filterBand );
        }

#pragma omp critical
        {
            if ( progress ) {
                progress->setPos ( ++done );
            }
        }
    }

   filtered.DeleteAll(); // free mem!

}

};

}