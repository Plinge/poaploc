#include  "wave/average.h"

using namespace Audio;

/**
  *  @param sampleCountAverage  samples to average over, whole file if  <0
  *  @param sampleStep          step to go through input file with
  */
void calcAverage(Wave& in,Wave & out,double sampleCountAverage,double sampleCountOut)
{
    int sampleCount = in.getFrameCount();
    int channelCount = in.getChannels();
    if (sampleCountOut<1.0) sampleCountOut = 1.0;
    out.create(in.getSamplingFrequency() / (double)sampleCountOut, in.getChannels(),
        ceil((in.getFrameCount()* in.getChannels() * 2) / sampleCountOut));
    int samplesOut = out.getFrameCount();
    double s2 = sampleCountAverage/2.0;
#pragma omp parallel for
    for (int sample=0;sample<samplesOut;sample++) {
        int x = sample*sampleCountOut;
        int s = (int)floor(0.5+x-s2);
        if (s<0) s=0;
        int e = (int)floor(0.5+x+s2);
        if (e>=sampleCount) e=sampleCount-1;
        double f =  1.0 / ((double)(e-s+1));
        for (int channel=0;channel<channelCount;channel++) {
            double sum = 0.0;
            for (int sub=s;sub<=e;sub++) {
                sum+=in.getSample(sub,channel);
            }
            out.setSample(sample,channel,sum*f);
        }
    }
}


/**
  *  @param sampleCountAverage  samples to average over, whole file if  <0
  *  @param sampleStep          step to go through input file with
  */

void calcAverageSparse(Wave& in,SparseWave & out,double sampleCountAverage,double sampleStep)
{
    int sampleCount = in.getFrameCount();
    int channelCount = in.getChannels();
    if (sampleCountAverage<0) {
        out.create(1.0 / in.getTotalSeconds(), in.getChannels(), 1 );
        sampleStep = in.getFrameCount()>>1;
    } else {
        if (sampleStep<1.0) sampleStep = 1.0;
        out.create(in.getSamplingFrequency() / (double)sampleStep, in.getChannels(), ceil(in.getFrameCount()/ sampleStep ) );
    }

    int samplesOut = out.getFrameCount();

//    LogFmtA("calcAverageSparse fin = %f (%d / %d) fout = %f (%d)",
//            (double)in.getSamplingFrequency(), (int)sampleCount, (int)sampleStep,
//            (double)out.getSamplingFrequency(), (int)samplesOut);

    double s2 = sampleCountAverage/2.0;
//#pragma omp parallel for
    for (int sample=0;sample<samplesOut;sample++) {
        int s,e;
        if (samplesOut==1) {
            s=0;
            e=sampleCount-1;
        } else {
            int x = sample*sampleStep;
            s = (int)floor(0.5+x-s2);
            if (s<0) s=0;
            e = (int)floor(0.5+x+s2);
            if (e>=sampleCount) {
                e=sampleCount-1;
            }
        }
        double f =  1.0 / ((double)(e-s+1));
        for (int channel=0;channel<channelCount;channel++) {
            double sum = 0.0;
            for (int sub=s;sub<=e;sub++) {
                sum+=in.getSample(sub,channel);
            }
            out.setSample(sample,channel,sum*f);
        }
    }
}
