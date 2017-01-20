#include "tabulartwodeebackprojection.h"


bool TablularTwoDeeBackprojection::calc(int idx,Audio::Wave & in, Audio::Wave & out)
{
    if (tables.isEmpty()) {
        qWarning("TablularTwoDeeBackprojection.calc() calling init() on the fly.");
#pragma omp critical
        {
            init();
        }

    }

    if (!init_done) {
       return false;
    }

    TimeDelayTable* table = &tables[idx];
    if (table==0) {
        throw std::runtime_error("Backprojection not initialized or no such index!");
    }
    int width = in.getFrameCount();
    int height = dimSub*dimSup;
    out.create(in.getSamplingFrequency(), height, width);
    int channels = in.getChannels();
    double mid = (channels-1)/2.0;
    for (int sample=0;sample<width;sample++) {
        register int channelOut=0;
        for (int x2=0;x2<dimSup;x2++) {
            //double *tdoas = table->at(x2);
            for (int x1=0;x1<dimSub;x1++) {
              //  double tdoa = *tdoas++;
                double tdoa = table->getValue(x2,x1);
                if (interpolate) {
                    register int c0 = (int)floor(tdoa+mid);
                    register double f = tdoa+mid-c0;
                    if (c0+1>=channels || c0<0)  {
                        qWarning("Backprojection TDOA %.2f out of channel range! %d channel(s), mid = %f.", tdoa, channels, mid);
                        qWarning("Channels %d,%d not in 0-%d! Table %d is %Dx%D, indices were %d,%d.", c0, c0+1, channels-1, idx, dimSub, dimSup, x1, x2);
                        throw std::runtime_error("Backprojection TDOA out of range!");
                    }
                    register double interp = in.getSample(sample,c0)*(1.0-f) + in.getSample(sample,c0+1)*(f);
                    out.setSample(sample, channelOut, interp);
                } else {
                   int c0 = (int)floor(tdoa+mid+0.5);
                   out.setSample(sample, channelOut, in.getSample(sample,c0));
                }
                channelOut++;
            }
        }
    }
    return true;
}

bool TablularTwoDeeBackprojection::calcFloat(int idx,Audio::FloatWave & in, Audio::FloatWave & out)
{
    if (tables.isEmpty() || !init_done) {
        throw std::runtime_error("Backprojection not initialized or no such index!");
    }
    TimeDelayTable* table = &tables[idx];
    if (table==0) {
        throw std::runtime_error("Backprojection not initialized or no such index!");
    }
    int width = in.getFrameCount();
    int height = dimSub*dimSup;
    out.create(in.getSamplingFrequency(), height, width);
    int channels = in.getChannels();
    float mid = (channels-1)/2.0F;
    float *inp  = in.getPtr();
    float *outp = out.getPtr();
    float *tdoas = table->at(0);
    if (interpolate) {
        for (int x=0;x<height;x++) {
            float tdoa = *tdoas++;
            int c0 = (int)floor(tdoa+mid);
            float f = tdoa+mid-c0;
            float g = 1.0F-f;
            float *ins  = inp;
            float *outs = outp++;
            for (int sample=0;sample<width;sample++) {
                *outs = ins[c0]*g+ins[c0+1]*f;
                ins += channels;
                outs += height;
            }
        }
    }  else {
        for (int x=0;x<height;x++) {
            float tdoa = *tdoas++;
            int ch = (int)floor(tdoa+mid+0.5);
            float *ins  = inp;
            float *outs = outp++;
            for (int sample=0;sample<width;sample++) {
                *outs = ins[ch];
                ins += channels;
                outs += height;
            }
        }
    }

    return true;
}

