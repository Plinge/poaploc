#include "poapifywave.h"
#include "assert.h"
#include <QElapsedTimer>
#include  <iostream>

using namespace std;

PoAPifyWave::PoAPifyWave(const Audio::Wave &wave) \
    :  bandsum(false)
{
    wav = &wave;
    setSamplingFrequency(wav->getSamplingFrequency());
}

bool PoAPifyWave::process(int sz, Audio::Wave &out,double startTime,double endTime)
{
    if (fabs(samplingFrequency - wav->getSamplingFrequency())>2.0) {
        qWarning( "Sampling rate mismatch!");
        return false;
    }

    frameCount = sz;

    precalc();
    createBuffers();

    int bufferSize = wav->getChannels() * sz;
    assert(bufferSize>0);
    float* buffer = (float*)malloc(bufferSize*sizeof(float));

    int maxFrame = wav->getFrameCount();
    bool continuous=false;
    if (endTime>=0.0) {
        maxFrame=(0.5+endTime*samplingFrequency);
    }
    int frame = startTime*samplingFrequency;
    int frameOut = 0;

    if (bandsum) {
        out.create(samplingFrequency,channels,maxFrame-frame);
    } else {
        out.create(samplingFrequency,channels*bandCount,maxFrame-frame);
    }

    QElapsedTimer timer;
    timer.start();
    while (frame<maxFrame) {
        float *po= buffer;
        int xx = bufferSize/channels;
        if (frame+xx>=maxFrame) {
            xx = maxFrame - frame;
        }
        for (int i=0;i<xx;i++) {
            for (int c=0;c<channels;c++) {
                *po++ =  wav->getSample(frame+i,c);
            }
        }

        calc(buffer, frameCount, continuous);

        updateGains();

        continuous=true;

        if (bandsum) {
            for (int channel=0;channel<channels;channel++) {
                for (int p=0;p<xx;p++) {
                    double v = 0.0;
                    for (int bandIndex=0;bandIndex<bandCount;bandIndex++) {
                        int spikedIndex = channels*bandIndex+channel;
                        v += spiked[spikedIndex]->getSample(p,0);
                    }
                    out.setSample(frameOut+p, channel, clip1(v));
                }
            }
        } else {
            for (int channel=0;channel<channels;channel++) {
                for (int bandIndex=0;bandIndex<bandCount;bandIndex++) {
                    int spikedIndex = channels*bandIndex+channel;
                    //int nz = spiked[spikedIndex]->getNozeroSampleCount();
                    //if (nz) {
                    //    cerr << channel << ", " << bandIndex << " " << nz << " spikes" << endl;
                    //}
                    for (int p=0;p<xx;p++) {
                        out.setSample(frameOut+p, spikedIndex, spiked[spikedIndex]->getSample(p,0));
                    }
                }
            }
        }
        frame +=  frameCount;
        frameOut += xx;
        if (!quiet) {
            double procTime = timer.elapsed()/1000.0;
            fprintf(stderr,"%.2fs processed in %.2fs (%.3f x real time)            \r",
                inputTime + frameTime, procTime, procTime / (inputTime+ frameTime));
        }
        if (debug && calcIteration>=debug) {
            break;
        }
        calcIteration++;
    }

    freeAll();
}

