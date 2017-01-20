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

#include "spikecorrelator.h"
#include  <qdebug.h>
#include <stdint.h>
#include <assert.h>

using namespace Neuro;

void SpikeWavCorrelator::setup(int wlen)
{
    cleanup();
    // LogFmtA("SpikeWavCorrelator.calc %D windows of type %d",wlen,windowTypeCode);
    window = (double*)malloc(wlen*sizeof(double));
    windowLength=wlen;

    const double a0=0.42;
    const double a1=0.5;
    const double a2=0.08;
    double fact1 = 2.0 * M_PI / (double) (windowLength - 1);
    double fact2 = 2.0 * fact1;
    window[0] = a0-a1+a2;
    for (int i = 1; i < windowLength; i++) {
            window[i] = a0 - a1 * cos(fact1 * (double)i) + a2 * cos(fact2 * (double)i);
            if (window[i]<0.0) {
                window[i]=0.0;
            }

    }

    re1  = (double*)malloc(windowLength*sizeof(double));
    re2  = (double*)malloc(windowLength*sizeof(double));
    val1 = (double*)malloc(windowLength*sizeof(double));
    idx1 = (int*)malloc(windowLength*sizeof(int));
    val2 = (double*)malloc(windowLength*sizeof(double));
    idx2 = (int*)malloc(windowLength*sizeof(int));
}

void SpikeWavCorrelator::cleanup()
{
    if (re1==0) return;
    free(re1); free(re2);
    free(idx1); free(val1);
    free(idx2); free(val2);
    re1=re2=0;
    if (window!=0) {
        free(window);
    }
    window=0;
    val1=val2=0;
    idx1=idx2=0;
}

void SpikeWavCorrelator::calcNormal(Audio::Wave& in1,int channel1, Audio::Wave& in2, int channel2, Audio::Wave& out, int step, int firstCenter,int samplesOut)
{
    int spikelen=3;
    //LogFmtA("SpikeWavCorrelator.calcNormal  %D channels over %D samples, step = %D / %.1fms, window = %D / %.1fms",channels, in1.getFrameCount(), step, (step*1e3 )/ in1.getSamplingFrequency(), windowLength, (windowLength*1e3)/in1.getSamplingFrequency() );

    long samplesIn = in1.getFrameCount();
    double fin = in1.getSamplingFrequency();
    if (fin<=0.0 || fin>999999999.0) {
        qWarning("SpikeWavCorrelator.calcNormal Illegal sampling rate %f !", fin);
    }
    if (samplesOut<1) {
        samplesOut=(((uint64_t)(samplesIn-windowLength)) / (uint64_t)step);
        if (samplesOut<1) samplesOut=1;
    }
    double tout = ((double)step*(double)samplesOut)/(fin);
    double fout = fin/(double)step;
    if (fout<=0.0 || fout>999999999.0) {
        qWarning("SpikeWavCorrelator.calcNormal Illegal sampling rate %f results form %f / %f!", fout, fin, (double)step);
    }
    if (tout<=0.0) {
        qWarning("SpikeWavCorrelator.calcNormal Illegal time %f results form %f *  %f / %f!", tout, (double) step, (double)samplesOut, fin);
    }
    if  (channels<=0) {
        qWarning("SpikeWavCorrelator.calcNormal Illegal channel count %d!", channels);
    }

    out.create(fout, channels, samplesOut);
    //LogFmtA("SpikeWavCorrelator.calcNormal fin = %f step = %d fout = %f.", in1.getSamplingFrequency(), step, out.getSamplingFrequency() );

    out.zero();
    re3 =  (double*)calloc(channels,sizeof(double));
    long sampleCountIn = in1.getFrameCount();
    long pos1 = 0;
    if (firstCenter>-9999) {
        pos1 = firstCenter - (windowLength>>1);
    }
    for (int index=0;index<samplesOut;index++) {
        double* wp = window;
        int p = pos1;
        for (int i=0;i<windowLength && p<sampleCountIn;i++) {
            double v1 = in1.getSample(p,channel1);
            double w = *wp++;
            re1[i] = v1*w;
            double  v2 = in2.getSample(p,channel2);
            re2[i] = v2*w;
            p++;
        }

        switch (mode)
        {
        case First:
        {
            int i1=-1,i2=-1;
            for (int i=0;i<windowLength;i++) {
                if (re1[i]>0.0 && i1<0)  {
                    i1=i;
                    if (i1>=0) break;
                }
                if (re2[i]>0.0 && i2<0)  {
                    i2=i;
                    if (i1>=0) break;
                }
            }
            setOutSpike(out, re1, re2, index, i1, i2);
        }
        break;

        case Max:
        {
            int i1=-1,i2=-1;
            double m1=0.0,m2=0.0;
            for (int i=0;i<windowLength;i++) {
                if (re1[i]>m1 || i1<0)  {
                    i1=i;
                    m1=re1[i];
                }
                if (re2[i]>m2 || i2<0)  {
                    i2=i;
                    m2=re2[i];
                }
            }
            setOutSpike(out, re1, re2, index, i1, i2);
        }
        break;

        case All:
        case Nearest:
        {

            double o1=0.0,o2=0.0;
            //LogFmtA("%d - %d",pos1,pos1+windowLength-1);
            int c=0,c2=0;

            for (int i=0;i<windowLength;i++) {
                double n1 = re1[i];
                if (o1<=0.0 && n1>0.0) {
                    idx1[c]=i;
                    val1[c]=n1;
                    //LogFmtA("ch1 %d %f",pos1+i,n1);
                    c++;
                }
                o1=n1;
            }
            for (int i=0;i<windowLength;i++) {
                double n2 = re2[i];
                if (o2<=0.0 && n2>0.0) {
                    idx2[c2]=i;
                    val2[c2]=n2;
                    //LogFmtA("ch2 %d %f",pos1+i,n2);
                    c2++;
                }
                o2=n2;
            }
            // LogFmtA("%d - %d found %d %d",pos1,pos1+windowLength-1,c,c2);

            for (int i=0;i<channels;i++) {
                re3[i]=0.0;
            }
            if (mode == Nearest) {
                for (int i1=0;i1<c;i1++) {
                    int dmin = windowLength;
                    int imin=-1;
                    for (int i2=0;i2<c2;i2++) {
                        int d = idx1[i1] - idx2[i2];
                        if (d>dmin) break;
                        if (abs(d)>dmin) continue;
                        imin =  i2;
                        dmin = abs(d);
                    }
                    if (imin<0) continue;
                    int channel = idx1[i1] - idx2[imin] + mid;
                    double v = val1[i1]*val2[imin];
                    double v2=v*0.66;
                    double v3=v*0.33;
                    if (validChannel(channel+2)) re3[channel+2] += v3;
                    if (validChannel(channel+1)) re3[channel+1] += v2;
                    if (validChannel(channel  )) re3[channel  ] += v;
                    if (validChannel(channel-1)) re3[channel-1] += v2;
                    if (validChannel(channel-2)) re3[channel-2] += v3;
                }
            } else {
                for (int i1=0;i1<c;i1++) {
                    for (int i2=0;i2<c2;i2++) {
                        int channel = idx1[i1] - idx2[i2] + mid;
                        double v = val1[i1] * val2[i2];
                        double v2=v*0.66;
                        double v3=v*0.33;
                        if (validChannel(channel+2)) re3[channel+2] += v3;
                        if (validChannel(channel+1)) re3[channel+1] += v2;
                        if (validChannel(channel  )) re3[channel  ] += v;
                        if (validChannel(channel-1)) re3[channel-1] += v2;
                        if (validChannel(channel-2)) re3[channel-2] += v3;
                    }
                }
            }
            for (int channel=0;channel<channels;channel++) {
                out.setSample(index,channel,re3[channel]);
            }
        }
        break;
        }
        pos1+=step;
    }
    free(re3);
}


void SpikeWavCorrelator::calcSpiked1(Audio::Wave& in1,int channel1, Audio::Wave& in2, int channel2, double* re3 , int pos1)
{
    int channelBorder = channels+2;
    int countNonzeros1=0;
    int hint1=-1;
    int winEnd = pos1+windowLength;
    double v;
    int indexNonzero1 = in1.getNextNonzeroIndex(pos1-1,channel1,hint1,v);
    while (indexNonzero1>=pos1 && indexNonzero1<winEnd) {
        assert(countNonzeros1<windowLength);
        assert(indexNonzero1-pos1>=0);
        assert(indexNonzero1-pos1<windowLength);
        val1[countNonzeros1] = v * window[indexNonzero1-pos1];
        if (val1[countNonzeros1]>0) {
            idx1[countNonzeros1]=indexNonzero1;
    //		LogFmtA("ch1 %d %f",i1,val1[c1]);
            countNonzeros1++;
        }
        if (countNonzeros1>=windowLength) {
            qWarning("More nonzeros than we can handle!");
            break;
        }
        indexNonzero1 = in1.getNextNonzeroIndex(indexNonzero1,channel1,hint1,v);
    };
    int countNonzeros2=0;
    int hint2=-1;
    int indexNonzero2 = in2.getNextNonzeroIndex(pos1-1,channel2,hint2,v);
    while (indexNonzero2>=pos1 && indexNonzero2<winEnd) {
        val2[countNonzeros2]= v * window[indexNonzero2-pos1];
        if (val2[countNonzeros2]>0) {
            idx2[countNonzeros2]=indexNonzero2;
    //		LogFmtA("ch2 %d %f",i2,val2[c2]);
            countNonzeros2++;
        }
        if (countNonzeros2>=windowLength) {
            qWarning("More nonzeros than we can handle!");
            break;
        }
        indexNonzero2 = in2.getNextNonzeroIndex(indexNonzero2,channel2,hint2,v);
    };
    //LogFmtA("%d - %d found %d %d",pos1,pos1+windowLength-1,c1,c2);
    for (int i=0;i<channels;i++) {
        re3[i]=0.0;
    }
    register int channel=0;
    for (int i2=countNonzeros2-1;i2>=0;i2--) {
        for (int i1=0;i1<countNonzeros1;i1++) {
            int p2 = idx2[i2];
            int p1 = idx1[i1];
            channel = p1 - p2 + mid;

            if (channel>channelBorder) {
                // indices crossed too far
            //    goto double_for_break2;
                break;
            }

            if (channel<-2) continue;

            double v = val1[i1] * val2[i2];
            if (v<=0.0) continue;
            double v2=v*0.66;
            double v3=v*0.33;
            if (validChannel(channel+2)) re3[channel+2] += v3;
            if (validChannel(channel+1)) re3[channel+1] += v2;
            if (validChannel(channel  )) re3[channel  ] += v;
            if (validChannel(channel-1)) re3[channel-1] += v2;
            if (validChannel(channel-2)) re3[channel-2] += v3;
        }
    }
}


void SpikeWavCorrelator::calcSpiked(Audio::Wave& in1,int channel1, Audio::Wave& in2, int channel2, Audio::Wave& out, int step,int firstCenter,int samplesOut)
{
    long samplesIn = in1.getFrameCount();
    // one more than full windows, might find something on the edge
    if (samplesOut<1)  {
        samplesOut = 1+(((uint64_t)(samplesIn-windowLength)) / (uint64_t)step);
        if (samplesOut<1) samplesOut=1;
    }
    //double tout = ((double)step*(double)samplesOut)/(in.getSamplingFrequency());
    double fout =  in1.getSamplingFrequency()/(double)step;
    out.create(fout, channels, samplesOut);
    out.zero();

   // LogFmtA("SpikeWavCorrelator.calcSpiked fin = %f fout = %f ", in1.getSamplingFrequency(), out.getSamplingFrequency() );
    if (in1.getDataSamples(channel1) == 0 || in2.getDataSamples(channel2)==0) {
        return;  // no data? then we are done ;-)
    }
    re3 =  (double*)malloc(channels*sizeof(double));
    long pos1 = 0;
    if (firstCenter>-9999) {
        pos1 = firstCenter - (windowLength>>1);
    }
    for (int index=0;index<samplesOut;index++) {
        calcSpiked1(in1,channel1,in2,channel2,re3,pos1);
        for (int channel=0; channel<channels; channel++) {
            out.setSample(index, channel, re3[channel]);
        }
        pos1+=step;
        if (pos1>samplesIn) {
            break;
        }
    }
    free(re3);
}


void SpikeWavCorrelator::calcOneForChannels(int channelsOut, Audio::Wave& in1, int channel1, Audio::Wave& in2, int channel2, double* result, int pos)
{
    channels = channelsOut;
    mid = channels >> 1;
    calcSpiked1(in1, channel1, in2,  channel2, result ,  pos);
}

bool SpikeWavCorrelator::calcForChannels(int channelsOut, Audio::Wave& in1, int channel1, Audio::Wave& in2, int channel2, Audio::Wave& out, int step, int firstCenter, int samplesOut)
{
    if (windowLength<1 || window==0) {
        throw std::runtime_error("SpikeWavCorrelator not initialized");
        return false;
    }
    channels = channelsOut;
    mid = channels >> 1;

    if (step<=0) {
        step = windowLength>>1;
    }
    if ((in1.isSparse() || in2.isSparse()) && mode == All ) {
      //  LogFmtA("Correlating %D channels %D frames / %D windows in sparse mode.", channels, in1.getFrameCount(), windowLength);
        calcSpiked(in1,channel1,in2,channel2,out,step,firstCenter,samplesOut);
        //LogFmtA("correlate %D x %D  => %D spikes in %D samples" , in1.getDataSamples(channel1),in2.getDataSamples(channel2),	out.getDataSamplesTotal(), out.getFrameCount() );
    } else {
      //  LogFmtA("Correlating %D channels %D windows in array mode, cor mode %d.", channels, windowLength, (int)mode);
        calcNormal(in1,channel1,in2,channel2,out,step,firstCenter,samplesOut);
    }
    if (out.getChannels()!= channels)   {
      qWarning("Correlation produced %d, not %d channels", out.getChannels(), channels);
    }
    return true;
}
