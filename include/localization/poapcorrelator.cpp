/*
 * Copyright 2010-2015 (c) Axel Plinge / TU Dortmund University
 *
 * ALL THE COMPUTER PROGRAMS AND SOFTWARE ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND.
 * WE MAKE NO WARRANTIES,  EXPRESS OR IMPLIED, THAT THEY ARE FREE OF ERROR, OR ARE CONSISTENT
 * WITH ANY PARTICULAR STANDARD OF MERCHANTABILITY, OR THAT THEY WILL MEET YOUR REQUIREMENTS
 * FOR ANY PARTICULAR APPLICATION. THEY SHOULD NOT BE RELIED ON FOR SOLVING A PROBLEM WHOSE
 * INCORRECT SOLUTION COULD RESULT IN INJURY TO A PERSON OR LOSS OF PROPERTY. IF YOU DO USE
 * THEM IN SUCH A MANNER, IT IS AT YOUR OWN RISK. THE AUTHOR AND PUBLISHER DISCLAIM ALL
 * LIABILITY FOR DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES RESULTING FROM YOUR USE OF THE
 * PROGRAMS.
 */

#include "poapcorrelator.h"
#include <iostream>

PoAPCorrelator::PoAPCorrelator(int t)
 : maxTdoa(t), bandsum(false)
{

}



void PoAPCorrelator::setCorrelationFraming(double len_ms, double step_ms, int t)
{

    maxTdoa=t;

    correlationSizeMs = len_ms;
    int correlationSize = samplingFrequency*len_ms*1e-3;
    if (step_ms > 0 ) {
        correlationStep = samplingFrequency*step_ms*1e-3;
    } else {
        correlationStep = -step_ms;
    }
    if (correlationStep<=0 || correlationSize<=0) {
        throw std::runtime_error("Parameter Error: Correlation step near zero!");
    } else {
        correlationCount = frameCount / correlationStep;
        if (correlationCount<1) {
            std::cerr << "step " << correlationStep << "size " << correlationSize << std::endl;
            throw std::runtime_error("Parameter Error: Correlation count zero!");
        }
    }
}

void PoAPCorrelator::initCorrelation()
{
    double frameSamples = correlationSizeMs*1e-3 *samplingFrequency;
    if (maxTdoa>0) {
        maxCorrelationValues=2*maxTdoa+1;
    } else {
        maxCorrelationValues=((int)(frameSamples/2))*2+1;
    }
    correlator = QSharedPointer< Neuro::SpikeWavCorrelator > (
                                      new Neuro::SpikeWavCorrelator(ceil(frameSamples),windowType)
                                      );

    correlationOverlap = ceil(frameSamples+1);
    nextCenterOffset=0;
    firstCenter=0;
}

void PoAPCorrelator::calc(const float *data, unsigned int frames, bool continued)
{
    PoAPifier::calc(data,frames,continued);

    int channels = getChannelCount();
    bandCount = filterbanks.first()->getCount();

    updateGains();

    firstCenter = correlationOverlap + nextCenterOffset; // - correlationStep;

    int correlationsPossible = (frameCount-(correlationOverlap>>1) - (firstCenter-correlationOverlap)) / correlationStep;
    correlationsPossible++;
    int nextCenter = firstCenter - correlationOverlap + correlationStep * (correlationsPossible);
    nextCenterOffset = nextCenter - frameCount;
//    qDebug("correlating over %5d .. %5d, %d correlation centers %5d .. %5d, overlap %d, next %d offset %d",
//            firstCenter-correlationOverlap/2,
//            firstCenter+correlationOverlap/2 + correlationStep * (correlationsPossible-1),
//            correlationsPossible,
//            firstCenter - correlationOverlap,
//            firstCenter - correlationOverlap + correlationStep * (correlationsPossible-1),
//            correlationOverlap,
//            nextCenter,
//            nextCenterOffset);

//    correlates.clear();


    if (bandCount>=(1<<12)) {
        throw std::runtime_error("too many bands");
    }
    if (channels>=(1<<10)) {
        throw std::runtime_error("too many channels");
    }

    if (bandsum) {

        for (int index1=0;index1<channels-1;index1++) {
            for (int index2=index1+1;index2<channels;index2++) {
                correlates[ (index1<<10) | (index2) ] = QSharedPointer<Audio::Wave>();
            }
        }

        for (int index1=0;index1<channels-1;index1++) {
            for (int index2=index1+1;index2<channels;index2++) {
                Audio::FloatWave * correlate = 0;
                for (int bandIndex=0;bandIndex<bandCount;bandIndex++) {
                    int spikedIndex = channels*bandIndex;
                    Audio::FloatWave * corrband = new Audio::FloatWave();
                    correlator->calcForChannels(maxCorrelationValues,
                         *spiked[index1 + spikedIndex], 0,
                         *spiked[index2 + spikedIndex], 0,
                         *corrband,
                         correlationStep, firstCenter, correlationsPossible);
                    if (!correlate) {
                        correlate = corrband;
                    } else {
                        for (int c=0;c<correlate->getChannels();c++) {
                            for (int s=0;s<correlate->getFrameCount();s++) {
                                correlate->setSample(s,c,correlate->getSample(s,c) + corrband->getSample(s,c));
                            }
                        }
                        delete corrband;
                    }
                }
                correlates[  (index1<<10) | (index2) ] = QSharedPointer<Audio::Wave>(correlate);
            }
        }

    } else {
        for (int bandIndex=0;bandIndex<bandCount;bandIndex++) {
            for (int index1=0;index1<channels-1;index1++) {
                for (int index2=index1+1;index2<channels;index2++) {
                    correlates[ (bandIndex<<20) | (index1<<10) | (index2) ] = QSharedPointer<Audio::Wave>();
                }
            }
        }


        for (int bandIndex=0;bandIndex<bandCount;bandIndex++) {
            int spikedIndex = channels*bandIndex;
            for (int index1=0;index1<channels-1;index1++) {
                for (int index2=index1+1;index2<channels;index2++) {
                    Audio::FloatWave * correlate = new Audio::FloatWave();
                    correlator->calcForChannels(maxCorrelationValues,
                         *spiked[index1 + spikedIndex], 0,
                         *spiked[index2 + spikedIndex], 0,
                         *correlate,
                         correlationStep, firstCenter, correlationsPossible);

                    correlates[ (bandIndex<<20) | (index1<<10) | (index2) ] = QSharedPointer<Audio::Wave>(correlate);

                }
            }
        }
    }
    calcIteration++;
}

