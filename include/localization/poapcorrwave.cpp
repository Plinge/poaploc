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

#include "poapcorrwave.h"
#include <assert.h>
#include <QElapsedTimer>
#include <iostream>

using namespace std;

PoAPCorrWave::PoAPCorrWave(const Audio::Wave &wave) : PoAPCorrelator(-1)
{
    wav = &wave;
    setSamplingFrequency(wav->getSamplingFrequency());
}



bool PoAPCorrWave::process(int sz, double startTime,double endTime)
{
    if (fabs(samplingFrequency - wav->getSamplingFrequency())>2.0) {
        qWarning( "Sampling rate mismatch!");
        return false;
    }

    initCorrelation();
    frameCount = sz;

    precalc();
    createBuffers();

    cout << "time" << "\t";
    if (!bandsum) {
        cout << "band\t";
    }
    cout << "mic0\t";
    cout << "mic1";
    for (int i=0;i<maxCorrelationValues;i++) {
        cout <<"\t" << "c" << i - maxCorrelationValues/2;
    }
    cout << endl;


    double sampleTime =  (double)correlationStep  / (double)samplingFrequency;

    int bufferSize = wav->getChannels() * sz;
    assert(bufferSize>0);
    float* buffer = (float*)malloc(bufferSize*sizeof(float));


    int maxFrame = wav->getFrameCount();
    if (endTime>=0.0) {
        maxFrame=(0.5+endTime*samplingFrequency);
    }
    //int maxPosition = maxFrame*channels;
    bool continuous=false;
    int frame=startTime*samplingFrequency;
    //unsigned int position=frame*channels;
    QElapsedTimer timer;
    timer.start();
    //while (position<maxPosition) {
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

        double overlapTime = (double)(firstCenter-correlationOverlap) / (double)samplingFrequency;

        QMapIterator<uint32_t, QSharedPointer<Audio::Wave> > i(correlates);
        i.next();
        int samples = i.value()->getFrameCount();

        if (debug) {
            fprintf(stderr,"Absolute cor %d .. %d + %d, total %d\n",
                    frame + firstCenter - correlationOverlap,
                    (int)(frame + firstCenter - correlationOverlap + correlationStep * (samples-1)),
                    (int)(correlationStep),
                    (int)samples
                   );
            fflush(stderr);
        }
        double frameStartTime = ((double)frame/(double)samplingFrequency);

        for (int sample=0;sample<samples;sample++) {
            i.toFront();

            while (i.hasNext()) {
                i.next();

                uint32_t bij = i.key();
                QSharedPointer< Audio::Wave > wave = i.value();

                cout << frameStartTime + overlapTime + sample*sampleTime << "\t";
                if (!bandsum) {
                    cout << ((bij>>20)            ) << "\t";
                }
                cout << ((bij>>10)&((1<<10)-1)) << "\t";
                cout << ((bij    )&((1<<10)-1));

                int chs = wave->getChannels();
                //assert(chs == maxCorrelationValues);
                for (int ch=0;ch<chs;ch++) {
                    cout << "\t" << wave->getSample(sample,ch) ;
                }
                cout << endl;
            }
        }

        //position += frameCount*channels;
        frame +=  frameCount;

        if (!quiet) {
            double procTime = timer.elapsed()/1000.0;
            fprintf(stderr,"%.2fs processed in %.2fs (%.3f x real time)            \r",
                    inputTime + frameTime, procTime, procTime / (inputTime+ frameTime));
            if (debug) {
                fprintf(stderr,"\n");
            }
        }
        if (debug && calcIteration>=debug) {
            break;
        }
        calcIteration++;
    }

    freeAll();
}

