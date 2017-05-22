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

#include "poapifier.h"
#include <iostream>
#include "wave/wavefile.h"

PoAPifier::PoAPifier()
    : samplingFrequency(44100), debug(0), correlationOverlap(0),
      frameCount(1024), inputTime(0.0), energyestimate(0.0), histogramTime(0.0)
{

}

PoAPifier::~PoAPifier()
{
    freeAll();
}

void PoAPifier::setSamplingFrequency(int f)
{
    samplingFrequency = f;
}

double PoAPifier::getSamplingFrequency()
{
    return samplingFrequency;
}

void PoAPifier::setGain(GainMode gm, double smooth, double ma)
{
    gainOffset = 0;
    gainMode = gm;
    gainSmooth = smooth;
    gainMax = ma;
}

double PoAPifier::getLastGain(int channel,int band)
{
    if (lastGain.isEmpty()) {
        return 0.0;
    }
    int index = channels*band+channel;
    if (index>=lastGain.count()) {
        return 0.0;
    }
    return lastGain.at(index);
}

void PoAPifier::precalc()
{

    inputTime=0;
    if (filterbanks.count()<1) {
        throw std::runtime_error("PoAPLocalizer Initialization Error: precalc() called with no filters!");
    }
    //LogFmtA("PoAPLocalizer.precalc gain mode %d gain %.3f", (int)gainMode, (double)addGain);
    bandCount = filterbanks.first()->getCount();

    channels = getChannelCount();
    fftOverlap = abs(filterbanks.first()->getOverlap());

    inputData.clear();
    for (int channel=0;channel<channels;channel++) {
        inputData.append( QVector<float>(frameCount + fftOverlap) );
    }
    lastSampleCountFiltered=0;
    calcIteration=0;

}

void PoAPifier::calc(const float *data, unsigned int frames, bool continued)
{
    unsigned int frameChannels = getChannelCount();
    channels = getChannelCount();

    if (!continued) {
        inputTime = 0.0;
    } else {
        inputTime += (double)frames / (double)samplingFrequency;
    }
    frameTime = (double)frames / (double)samplingFrequency;
    if (inputData.count()!=channels) {
        throw std::runtime_error("Buffer inputData misconfigured!");
    }
    int framePos = 0;
    // overlap
    if (continued && calcIteration>0) {
        framePos = fftOverlap;
#pragma omp parallel for
        for (int channel=0;channel<channels;channel++) {
            float* inPtr = inputData[channel].data();
            float* overlapPtr = inPtr + lastSampleCountFiltered - fftOverlap;
            memcpy(inPtr, overlapPtr, fftOverlap*sizeof(float));
        }
    }
    int sampleCountFiltered = frames+framePos;
    lastSampleCountFiltered = sampleCountFiltered;

    processData(data, framePos, frameChannels, sampleCountFiltered, continued);

}

double PoAPifier::getInputTime() const
{
    return inputTime;
}

double PoAPifier::getFrameLength() const
{
    return frameTime;
}

int PoAPifier::getBandCount() const
{
    return bandCount;
}

QVector<double> PoAPifier::getCenterFrequencies() const
{
    return filterbanks.first()->getFrequencies();
}

void PoAPifier::createBuffers()
{
    bandCount = filterbanks.first()->getCount();
    fastPoA = false; //(poaDelay < (-(int)correlationStep));
    spiked.clear();
    spiked.reserve(channels*bandCount);
    // spike buffers
    while (spiked.size()<channels*bandCount) {

        spiked.append(QSharedPointer<Audio::SparseWave>(
                          new Audio::SparseWave(samplingFrequency, 1, frameCount + correlationOverlap))
                      );
        if (!fastPoA) {
            averaged.append(QSharedPointer<Audio::FloatWave> (
                                new Audio::FloatWave(samplingFrequency, 1, abs(poaDelay)))
                            );
            averages.append(QSharedPointer<MovingAverage>(
                                new MovingAverage(poaAverage))
                            );
        }


    }

    //    qDebug("created %d sparse waves for spiked data.",spiked.size());
    if (!spikers.isEmpty()) {
        for (int i=0;i<spikers.size();i++) {
            spikers[i]->reset();
        }
        Neuro::SpikeGeneratorPoaPrecedence* spike = spikers.first().data();
        while (spikers.size()<channels*bandCount) {
            spikers.append(QSharedPointer< Neuro::SpikeGeneratorPoaPrecedence> (
                               new Neuro::SpikeGeneratorPoaPrecedence(*spike)));
        }

        //    qDebug("created %d spikers.",spikers.size());
    }

    while (buffers.size()<channels) {
        buffers.append( QVector<float>( (frameCount+fftOverlap) *  bandCount));
    }

    //    qDebug("created %d buffers.",buffers.size());

    if (gainMode != OFF  && addGain > 0.0) {
        double gg = 20.0*log10(addGain);
        int forget;
        if (histogramTime>0) {
            forget = (int)(samplingFrequency*histogramTime);
        } else {
            forget = (int)(samplingFrequency/(4.0*frameCount));
        }
        while (histograms.size()<channels*bandCount) {
            histograms.append(QSharedPointer<SparseHistogram>(
                                  new SparseHistogram(-180+gg, 0+gg, 256, 1024, forget))
                              );
        }
        if (debug) {
            qDebug("Historgrams %f %f %d %d %d", -180+gg, 0+gg, 256, 1024, forget);
        }
        for (int index=0;index<channels*bandCount;index++) {
            spikers[index]->attachHistogram(histograms[index]);
            histograms[index]->reset();
        }

        //        qDebug("created %d histograms.",histograms.size());

    }


}

void PoAPifier::setLastGain(int channel,int band,double g)
{
    int index  = channels*band+channel;
    if (lastGain.size()<=index) {
        lastGain.resize(index+1);
    }
    lastGain[index]=g;
}

void PoAPifier::resetLastGain()
{
    int n =channels*bandCount;
    lastGain.resize(n);
    for (int i=0;i<n;i++) {
        lastGain[i]=gainOffset;
    }
}

void PoAPifier::setFilter(int n,int f1,int fn,double gg, double scale)
{
    //    if (correlationStep<=0 || correlationCount<=0) {
    //        throw std::runtime_error("Misconfiguration");
    //    }
    filterbanks.clear();
    //int nu = bitutil::msb(windowSize*windowCount);
    //LogFmtA("Using nu=%d for %d samples", nu, windowCount*windowSize);
    for (int i=0;i<getChannelCount();i++) {
        AuditoryFilterbankF* filterbank = new AuditoryFilterbankF(n, f1, fn, samplingFrequency, scale);
        filterbank->setGainMode(0);
        //filterbank->setNu(9);
        filterbanks.append(QSharedPointer<AuditoryFilterbankF>(filterbank));
    }

    addGain = pow(10.0, gg / 20.0);
    bandCount = n;
}



double mygain(double f)
{
    if  (f<860.0)
        return f*18.0/860.0;
    if (f>2400.0)
        return 30.0;
    return 18.0+(f-860.0)*12/(2400.0-860.0);
}

void PoAPifier::updateGain(int bandIndex,int channel,double gain)
{
    if (gain>gainMax) {
        //        LogFmtA("Gain %d %d exceeds maximum with %f!", bandIndex, channel, gain);
        gain = gainMax;
    }

    if (gainSmooth>0) {
        gain = (gainSmooth)*gain + (1.0-gainSmooth)*getLastGain(channel,bandIndex);
    }

    int spikedIndex = channels*bandIndex+channel;
    double volume = pow(10.0,gain/20.0);
    spiked [spikedIndex]->multiplyBy(volume, 0);
    spikers[spikedIndex]->setEnergyThreshold(absoluteSpikeThreshold * volume);
    setLastGain(channel,bandIndex,gain);
}


void PoAPifier::updateGains()
{
    channels =getChannelCount();
    bandCount = filterbanks.first()->getCount();

    if (gainMode != OFF) {
        double mean = 0.0;
        for (int bandIndex=0;bandIndex<bandCount;bandIndex++) {
            for (int channel=0;channel<channels;channel++) {
                int spikedIndex = channels*bandIndex+channel;
                mean += histograms[spikedIndex]->probLeValue(0.95);
            }
        }
        mean /= (double)(channels*bandCount);
        energyestimate = mean;
    }

    if (gainMode == BANDS) {
        // band  mode for now
        //        cerr << "gain per band: ";
        for (int bandIndex=0;bandIndex<bandCount;bandIndex++) {
            double gain = 0;
            for (int channel=0;channel<channels;channel++)   {
                int spikedIndex = channels*bandIndex+channel;
                //                LogFmtA("channel %2d band %2d spikes %4d. Histogram spikes = %d %d %d mean = %f 0.95 = %f 0.05 = %f",
                //                    channel, bandIndex,
                //                    spiked[spikedIndex].getDataSamplesTotal(),
                //                    histograms[spikedIndex].getSampleCountUnder(),
                //                    histograms[spikedIndex].getSampleCount(),
                //                    histograms[spikedIndex].getSampleCountOver(),
                //                    histograms[spikedIndex].getMean(),
                //                    histograms[spikedIndex].probLeValue(0.95),
                //                    histograms[spikedIndex].probLeValue(0.05)
                //                    );
                gain += -histograms[spikedIndex]->probLeValue(0.95);
            }
            gain /= (double)(channels);
            gain += gainOffset;
            //            LogFmtA("band %2d auto gain = %fdB (offst %.2f)",bandIndex,gain,gainOffset);
            for (int channel=0;channel<channels;channel++) {
                updateGain(bandIndex,channel,gain);
            }
        }
    }

    if (gainMode == EMPF_AUTO) {
        double gain = 0;
        for (int bandIndex=0;bandIndex<bandCount;bandIndex++) {
            for (int channel=0;channel<channels;channel++) {
                int spikedIndex = channels*bandIndex+channel;
                gain += -histograms[spikedIndex]->probLeValue(0.95);
            }
        }
        gain /= (double)(channels*bandCount);
        gain += gainOffset;
        for (int bandIndex=0;bandIndex<bandCount;bandIndex++) {
            double f = filterbanks[0]->getFrequency(bandIndex);
            double em = mygain(f);
            for (int channel=0;channel<channels;channel++) {
                updateGain(bandIndex,channel,em+gain);
            }
            //            LogFmtA("band %2d empf auto gain = %fdB (empf %.2f)",bandIndex,gain+em,em);
        }
        // cerr << "energy " << energyestimate << " gain " << gain << " avg " << lastGain.calcAverage() << endl;
    }

    if (gainMode == AUTO) {
        double gain = 0;
        for (int bandIndex=0;bandIndex<bandCount;bandIndex++) {
            for (int channel=0;channel<channels;channel++) {
                int spikedIndex = channels*bandIndex+channel;
                gain +=  -histograms[spikedIndex]->probLeValue(0.95);
            }
        }
        gain /= (double)(channels*bandCount);
        gain += gainOffset;
        for (int bandIndex=0;bandIndex<bandCount;bandIndex++) {
            for (int channel=0;channel<channels;channel++) {
                updateGain(bandIndex,channel,gain);
            }
            //            LogFmtA("band %2d auto gain = %fdB",bandIndex,gain);
        }
    }

}


void PoAPifier::setSpikes(double pre,double len,double gain,double th,double tha,int mode)
{
    poaDelay =  pre *1e-3 * samplingFrequency;
    poaAverage = len * 1e-3 * samplingFrequency;
    absoluteSpikeThreshold = tha;
    spikers.clear();
    if (mode==0) {
        return ;
    }
    Neuro::SpikeGeneratorPoaPrecedence* spike = new  Neuro::SpikeGeneratorPoaPrecedence();
    spike->setAverageTime(len);
    spike->setShiftTime(pre);
    spike->setRelativeThreshold(th);
    spike->setEnergyThreshold(tha);
    spike->setGain(gain);
    spike->setHalfway(mode&1 == 1);
    if (mode <3) {
        spike->setMode(Neuro::SpikeGeneratorPoaPrecedence::POAP);
    } else if (mode < 5) {
        spike->setMode(Neuro::SpikeGeneratorPoaPrecedence::ZC);
    } else {
        spike->setMode(Neuro::SpikeGeneratorPoaPrecedence::HWR);
    }
    spikers.append( QSharedPointer<Neuro::SpikeGeneratorPoaPrecedence>(spike) );
}


void PoAPifier::process(int channel, int framePos, int frameChannels, int sampleCountFiltered, bool continued)
{
//    if (debug && (!continued || framePos == 0)) {
//        LogFmtA("PoAPLocalizer::process RESET spikes.");
//    }
    int samplesOut = frameCount-fftOverlap+framePos;
    // filter
    filterbanks[channel]->calcBuffer(sampleCountFiltered, inputData[channel].data(), buffers[channel].data(), framePos>0 ? 0 : Filtering::HAVE_START);
    // copy, spike, emphasize
    float* bufptr= buffers[channel].data();

    if (debug && calcIteration>=debug) {
        Audio::WaveFile w;
        w.create(samplingFrequency,bandCount,samplesOut);
        for (int bandIndex=0;bandIndex<bandCount;bandIndex++) {
            float* filtered = bufptr+sampleCountFiltered*bandIndex;
            for (int i=0;i<samplesOut;i++) {
                w.setSample(i,bandIndex,*filtered++);
            }
        }
        QString s = QString("PF_%1F%2_Bands.wav").arg(channel).arg(calcIteration);
        w.write(s);

        w.create(samplingFrequency,1,frameCount);
        for (int i=0;i<frameCount;i++) {
            w.setSample(i,0,inputData[channel].at(i));
        }
        s = QString("PF_%1I%2_Input.wav").arg(channel).arg(calcIteration);
        w.write(s);

       // std::cerr << "calc iteration " << calcIteration << " debug " << debug << std::endl;
    }

    if (spikers.isEmpty()) {
        for (int bandIndex=0;bandIndex<bandCount;bandIndex++) {
            float* filtered = bufptr+sampleCountFiltered*bandIndex;
            int spikedIndex = channels*bandIndex+channel;
            for (int sampleIndex=0;sampleIndex<sampleCountFiltered;sampleIndex++) {
                spiked[spikedIndex]->setSample(sampleIndex,0,*filtered++);
            }
        }
        return;
    }

    for (int bandIndex=0;bandIndex<bandCount;bandIndex++) {
        float* filtered = bufptr+sampleCountFiltered*bandIndex;
        int spikedIndex = channels*bandIndex+channel;
        if (correlationOverlap<1) {
           spiked[spikedIndex]->zero();
        } else {
           /// undo gain
           spiked[spikedIndex]->shiftBy(frameCount);
           double g = getLastGain(channel,bandIndex);
           if (g!=0.0) {
//               if (channel==0 &&  bandIndex==0) {
//                   LogFmtA("undoing gain %f", g);
//               }
               spiked[spikedIndex]->multiplyBy(pow(10.0,-g/20.0));
           }
        }
        if (gainMode != OFF) {
            histograms[spikedIndex]->step();
        }
        //LogFmtA("Using trace %d for channel %d band %d", spikedIndex, channel,bandIndex);
        if (fastPoA) {
            // calc PoA based on window energy (lower quality)
            double avg =0.0;
            for (int sampleIndex=0;sampleIndex<samplesOut;sampleIndex++) {
                avg += fabs( *(filtered+sampleIndex) );
            }
            avg = avg / (double) samplesOut;
            spikers[spikedIndex].reset(); // treat every window as start of data!
            for (int sampleIndex=0;sampleIndex<samplesOut;sampleIndex++) {
                spikers[spikedIndex]->update(*spiked[spikedIndex], 0, sampleIndex+correlationOverlap, *(filtered+sampleIndex), avg);
            }
        } else {
            if (poaDelay<0) {
                if (!continued || framePos == 0) { // calcIteration == 0
                    averages[spikedIndex]->Reset();
                    spikers[spikedIndex]->reset();
                    for (int sampleIndex=0;sampleIndex<-poaDelay;sampleIndex++) {
                        averages[spikedIndex]->Calc(fabs( *(filtered+sampleIndex)));
                    }
                } else {
                    for (int sampleIndex=0;sampleIndex<-poaDelay;sampleIndex++) {
                        spikers[spikedIndex]->update(*spiked[spikedIndex], 0, sampleIndex+correlationOverlap,
                            averaged[spikedIndex]->getSample(sampleIndex,0),
                            averages[spikedIndex]->Calc(fabs( *(filtered+sampleIndex))));
                    }
                }
                int sampleIndexOut = 0;
                for (int sampleIndex=-poaDelay;sampleIndex<samplesOut;sampleIndex++,sampleIndexOut++) {
                    spikers[spikedIndex]->update(*spiked[spikedIndex], 0, sampleIndex+correlationOverlap,
                        *(filtered+sampleIndexOut),
                        averages[spikedIndex]->Calc(fabs(*(filtered+sampleIndex)))
                        );
                }
                // precalc for next frame with overlap
                int sampleIndexOverlap=0;
                for (;sampleIndexOut<samplesOut;sampleIndexOverlap++,sampleIndexOut++) {
                    averaged[spikedIndex]->setSample(sampleIndexOverlap++, 0,
                        averages[spikedIndex]->Calc(fabs(*(filtered+sampleIndexOut)))
                    );
                }
            } else {
                if (!continued || framePos == 0) {
                    averages[spikedIndex]->Reset();
                    spikers[spikedIndex]->reset();
                    // calc poaDelay samples form signal and 0.0 as average
                    for (int sampleIndex=0;sampleIndex<poaDelay;sampleIndex++) {
                        spikers[spikedIndex]->update(*spiked[spikedIndex], 0, sampleIndex+correlationOverlap,
                            *(filtered+sampleIndex),
                            0.0
                            );
                    }
                } else {
                    // use average from previous frame
                    for (int sampleIndex=0;sampleIndex<poaDelay;sampleIndex++) {
                        spikers[spikedIndex]->update(*spiked[spikedIndex], 0, sampleIndex+correlationOverlap,
                            *(filtered+sampleIndex),
                            averaged[spikedIndex]->getSample(sampleIndex,0)
                            );
                    }
                }
                // calc rest with average on poaDelay older input signal
                int sampleIndexIn = 0;
                for (int sampleIndex=poaDelay;sampleIndex<samplesOut;sampleIndex++,sampleIndexIn++) {
                    spikers[spikedIndex]->update(*spiked[spikedIndex],
                        0, sampleIndex+correlationOverlap,
                        *(filtered+sampleIndex),
                        averages[spikedIndex]->Calc(fabs( *(filtered+sampleIndexIn)))
                        );
                }
                // precalc for next frame with overlap
                int sampleIndexOverlap = 0;
                for (;sampleIndexIn<samplesOut;sampleIndexIn++) {
                    averaged[spikedIndex]->setSample(sampleIndexOverlap++, 0,
                        averages[spikedIndex]->Calc(fabs( *(filtered+sampleIndexIn)))
                        );
                }
            }
        }
        /* emphasis
        double f = filterbanks[channel].getFrequency ( bandIndex );
        double gain = 1.0;
        if ( f>1000.0 ) {
            gain = ( log ( f/1000.0 ) / log ( 2.0 ) );
        }
        spiked[spikedIndex].multiplyBy( pow ( 10.0,gain/20.0 ), 0);
        */

        int nz = spiked[spikedIndex]->getNonzeroSampleCount();
        if (debug && nz>0) {
            double pr = (1000.0 * nz) / ((double)spiked[spikedIndex]->getFrameCount() * (double)spiked[spikedIndex]->getChannels());
            qDebug("Mic %d Band %2d %4.2fKHz spikes: Got %9d (0<x<%.3f) %4.1f/1000 in %.3fs = %d.",
                    channel, bandIndex,  filterbanks[0]->getFrequency(bandIndex) *1e-3, nz, spiked[spikedIndex]->getMaxSample(), pr,
                    spiked[spikedIndex]->getTotalSeconds(), spiked[spikedIndex]->getFrameCount() );
        }
    }

    if (debug && calcIteration>=debug) {
        Audio::WaveFile w;
        int bandIndex=0;
        int spikedIndex = channels*bandIndex+channel;
        w.create(spiked[spikedIndex]->getSamplingFrequency(),
                 bandCount,
                 spiked[spikedIndex]->getFrameCount());
        for (bandIndex=0;bandIndex<bandCount;bandIndex++)  {
            spikedIndex = channels*bandIndex+channel;
            spiked[spikedIndex]->copyChannelTo(&w,0,-1,bandIndex);
        }
        //w.normalize();
        //w.write("PF_0Spiked.wav");
        QString s = QString("PF_%1Spiked.wav").arg(channel);
        w.write(s);
       // std::cerr << "calcIteration " << calcIteration << std::endl;
    }
}

void PoAPifier::processData(const float* data, int framePos, int frameChannels, int sampleCountFiltered, bool continued)
{
#pragma omp parallel for
    for (int channel=0;channel<channels;channel++) {
        const float* dataPtr = data+channel;
        float* inPtr = inputData[channel].data();
        if (inPtr==0) {
            throw std::runtime_error("Buffer inputData not allocated!");
        }
        inPtr += framePos;
        for (int frame=0;frame<frameCount;frame++) {
            *inPtr++ = *(dataPtr) * addGain;
            dataPtr += frameChannels;
        }
        process(channel,framePos,frameChannels,sampleCountFiltered,continued);
    }
}

void PoAPifier::freeAll()
{
    averages.clear();
    averaged.clear();
    spiked.clear();
    spikers.clear();
    buffers.clear();
    inputData.clear();

    filterbanks.clear();
    histograms.clear();
}


//void PoAPifier::processData(const short* data, int framePos, int frameChannels, int sampleCountFiltered, bool continued)
//{
//#pragma omp parallel for
//    for (int channel=0;channel<channels;channel++) {
//        const short* dataPtr = data+channel;
//        float* inPtr = inputData[channel].data() + framePos;
//        for (int frame=0;frame<frameCount;frame++) {
//            *inPtr++ = *(dataPtr)  * addGain * (1.0F/32768.0F);
//            dataPtr += frameChannels;
//        }
//        process(channel,framePos,frameChannels,sampleCountFiltered,continued);
//    }
//}

//void PoAPifier::processData(const long* data, int framePos, int frameChannels, int sampleCountFiltered, bool continued)
//{
//#pragma omp parallel for
//    for (int channel=0;channel<channels;channel++) {
//        const long* dataPtr = data+channel;
//        float* inPtr = inputData[channel].data() + framePos;
//        for (int frame=0;frame<frameCount;frame++) {
//            *inPtr++ = *(dataPtr)  * addGain * (1.0F/32768.0F) * (1.0F/65536.0F);
//            dataPtr += frameChannels;
//        }
//        process(channel,framePos,frameChannels,sampleCountFiltered,continued);
//    }
//}

