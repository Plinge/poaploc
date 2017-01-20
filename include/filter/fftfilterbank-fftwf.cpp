#include "fftfilterbank-fftwf.h"
#include <stdexcept>
#include <assert.h>

FftFilterbankFFTWF::FftFilterbankFFTWF(int bandCount, double fmin, double fmax, double samplingFrequency, Filtering::Spacing s, Filtering::FilterShape m,double scaling)
: FftFilterbank<float>(bandCount, fmin, fmax, samplingFrequency, s, m, scaling), planned(false)
{
    nu=10;    
    FftFilterbank<float>::init(); // initFilters();
    normalizeFactor =  1.0F/length;
    initBuffers();
    plan();
}

FftFilterbankFFTWF::~FftFilterbankFFTWF()
{
    clearBuffers();
    unplan();
}

void FftFilterbankFFTWF::plan()
{
    unplan();    
#pragma omp critical
    {
        fftwf_complex *fi,*fo;
        fi = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * length);
        fo = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * length);
        plan_dft = fftwf_plan_dft_1d(length, fi, fi, FFTW_FORWARD, FFTW_MEASURE);
        plan_idft = fftwf_plan_dft_1d(length, fo, fo, FFTW_BACKWARD, FFTW_MEASURE);
        fftwf_free(fi);
        fftwf_free(fo);
    }
    planned=true;
    //LogFmtA("FftFilterbankFFTWF.planed length=%d", length);
}

void FftFilterbankFFTWF::unplan()
{
    if (planned) {
#pragma omp critical
        {
            fftwf_destroy_plan(plan_dft);
            fftwf_destroy_plan(plan_idft);

        }
    }
    planned=false;
    //LogMsgA("FftFilterbankFFTW.unplanned");
}


void FftFilterbankFFTWF::initBuffers()
{
    overhang.clear();
    for (int i=0;i<count;i++) {
        overhang.append(QVector<float>(length2));
    }
}

void FftFilterbankFFTWF::init()
{
    FftFilterbank<float>::init(); // initFilters();
    normalizeFactor =  1.0F/length;
    initBuffers();
    if (!planned) {
        plan();
    }
}

void FftFilterbankFFTWF::setNu(int nn)
{
    if (nn==nu) return;
//    LogFmtA("FftFilterbankFFTWF changing nu to %d", nn);
    unplan();
    nu = nn;
    init();
}

void FftFilterbankFFTWF::clear()
{
    clearBuffers();
}

void FftFilterbankFFTWF::clearBuffers()
{
    overhang.clear();
}

float* FftFilterbankFFTWF::calcBufferSlice(unsigned int sampleCount,float* in,float* out,int bandStart,int bandEnd,int positionInfo)
{

    if (!planned) {
        throw std::runtime_error("fftw not planned!");
    }
 
    fftwf_complex *fIn,*fOut;
    fIn = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * length);
    fOut = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * length);

    memset(out,0,sampleCount*sizeof(float)*(bandEnd-bandStart));    
//     unsigned int sz = sampleCount*(bandEnd-bandStart);
//     float* outptr = out;
//     for (int i=0;i<sz;i++) {
//         *outptr++ = 0.0F;
//     }
    
    if ((positionInfo&Filtering::HAVE_START)==0) {
//        LogFmtA("FftFilterbankFFTW Using %D samples overhang" , length2);
        long bandOffset=0;
        for (int band=bandStart;band<bandEnd;band++) {
            float* outptr = out+bandOffset;
            if (overhang[band].count()<length2) {
                qWarning("FftFilterbank.calcBufferSlice WTF!");
                continue;
            }
            float* ooptr = overhang[band].data();
            for (int sample=0;sample<length2;sample++) {
                *outptr++ = *ooptr++;
            }
            bandOffset+=sampleCount;
        }
    }
    if ((positionInfo&Filtering::HAVE_END)==0) {
        if ( (sampleCount/length2)*length2 != sampleCount) {
            qWarning("FftFilterbankFFTW Nonfitting sample count %d for overlap strategy %d ..", sampleCount, length2);
        }
    }
    
    int position = 0;
    int maxpos   = length2*((sampleCount/length2)-2);    
    float* inptr = in;
    while (position<=maxpos) {
        float* w = window.data();
        int sample;
        for (sample=0;sample<length;sample++) {
            fIn[sample][0] = inptr[sample] * *w++;
            fIn[sample][1] = 0.0;
        }
        assert( (position+sample)<= sampleCount );
//        if ((position+sample) > sampleCount) {
//            throw std::runtime_error("FftFilterbankFFTW provoked havoc!");
//        }
        //LogFmtA("FftFilterbankFFTW Calculating for %D - %D" ,position , position+length);
        fftwf_execute_dft(plan_dft, fIn, fIn);
        long bandOffset = 0;
        for (int band=bandStart;band<bandEnd;band++) {
            QVector<float>* cc = &coeffs[band];
            if (cc==0) {
                throw std::runtime_error("FftFilterbankFFTW messed up!");
            }
            float* c = cc->data();
            for (int tap=0;tap<length;tap++) {
                fOut[tap][0] = fIn[tap][0] * *c;
                fOut[tap][1] = fIn[tap][1] * *c;
                c++;
            }
            fftwf_execute_dft(plan_idft, fOut, fOut);
            float* outptr = out + position + bandOffset;
            for (sample=0;sample<length;sample++) {                
                *outptr++ += fOut[sample][0]* normalizeFactor;
            }
            assert( (position+sample)<= sampleCount );
            bandOffset+=sampleCount;
        }
        position += length2;
        inptr += length2;
    }
    position -= length2;
//    if (position < maxpos) {
//        LogFmtA("W_ FftFilterbankFFTW ended short! Used %D / %D samples" , position+length, sampleCount);
//    } else if (position > maxpos) {
//        LogFmtA("E_ FftFilterbankFFTW ended odd! Used %D / %D samples" , position+length, sampleCount);
//        throw std::runtime_error("FftFilterbankFFTW ended odd!");
//    } else {
//        LogFmtA("FftFilterbankFFTW ok. Used %D / %D samples in %D steps size %D." , position+length, sampleCount, length2, length);
//    }
    if (position > maxpos) {
        throw std::runtime_error("FftFilterbankFFTW ended odd!");
    }
    if ((positionInfo&Filtering::HAVE_END)==0) {
//       LogFmtA("FftFilterbankFFTW ok. Used %D / %D samples, memorizing %D from %D for overhang" , position+length, sampleCount,length2, maxpos+length2);
        long bandOffset=0;
        for (int band=bandStart;band<bandEnd;band++) {
            float* outptr = out + maxpos + length2 + bandOffset;
            float* ooptr = overhang[band].data();
            for (int sample=0;sample<length2;sample++) {
                *ooptr++ = *outptr++;
            }
            bandOffset+=sampleCount;
        }
    }
    fftwf_free(fIn);
    fftwf_free(fOut);
    return  out;
}
