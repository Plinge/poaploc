#include "micarr/poaplocalizercircular.h"

PoAPLocalizerCircular::PoAPLocalizerCircular()
    : backProCirc(0)
{

}

PoAPLocalizerCircular::~PoAPLocalizerCircular()
{

}

void PoAPLocalizerCircular::setMic(double r,int n,double a)
{
    if (n!=8) {
        throw new std::runtime_error("Mic count must be eight!");
    }
    mic.setRadius(r); mic.setCount(n); mic.setAlternatingHeight(a);
}

void PoAPLocalizerCircular::setCombination(CircularMicArrayCorrelator:: CombinationStrategy sel,double g)
{
    mic.setMicConfig(sel);
    combination  = new HamacherCombination(g);
}

int PoAPLocalizerCircular::getMicrophoneCount()
{
    return mic.getMicrophoneCount();
}

AzimuthElevationBackprojection* PoAPLocalizerCircular::getBackProjection()
{
    return backProCirc;
}

int PoAPLocalizerCircular::getAngularCount()
{
    if (!backProCirc) return 360;
    return backProCirc->getDim1Size();
}

void PoAPLocalizerCircular::correlatePairs(int bandIndex,int offset,int step,Audio:: WavGroup& correlates,int start,int end)
{
    double distance = mic.getMicDistance( offset);
    double maxSample = ceil(distance / sampleDistance)+1;
    if (maxSample < 3) {
//      LogFmtA("W_ correlate skipping %d-pairs, only %.0f samples.", offset, maxSample);
        return;
    }
    if (spatialLimit < 99.0 && spatialLimit > 0.0) {
        double f = filterbanks.GetHead()->getFrequency(bandIndex);
        f += 2.0 * Hearing::Gammatones::bandwidthGlasbergMoore(f);
        double freqSample  = ceil(samplingFrequency/f);
        double aliasSample = ceil(spatialLimit * freqSample);
        if (aliasSample < maxSample) {
//      LogFmtA("correlate skipping %d-pairs, %.2fkHz below alias theshold of %.2f (%.0f < %.0f samples)", offset, f*1e-3, spatialLimit, aliasSample, maxSample);
            return;
        }
    }
    //Audio::FloatWave correlate;
    Audio::AP_wav correlate;
    int chs = 2* ( ceil ( distance / sampleDistance ) + 1 );
    Neuro::SpikeWavCorrelator* cor = correlator.GetPtrByKey(bandIndex*4+offset-1);
    if (cor == 0) {
        throw new std::runtime_error("No correlator object!?");
    }
    int channels = mic.getMicrophoneCount();
    int spikedIndex = channels*bandIndex;
    for (int index=start;index<end;index+=step) {
        cor->calcForChannels(chs, spiked[((index+offset)&7) + spikedIndex], 0, spiked[(index+0) + spikedIndex], 0, correlate, correlationStep );
        Audio::AP_wav* p = new Audio::AP_wav();
        //Audio::FloatWave* p =  new  Audio::FloatWave();
        backProCirc->calcPair((index+offset)&7, index, correlate, *p);
        correlates.AddTail(p);
    }
}

void PoAPLocalizerCircular::correlate(bool continued)
{
#pragma omp parallel for
    for (int bandIndex=0;bandIndex<bandCount;bandIndex++) {
        double freq = filterbanks.GetHead()->getFrequency (bandIndex);
        Audio::WavGroup correlates;
        if (freq < samplingFrequency*0.5) { // skip aliased bands!
            switch (mic.getMicConfig()) {
            case CircularMicArray::Pair:
                correlatePairs( bandIndex, 4, 2, correlates,0 ,1 );
                break;
            case CircularMicArray::Four:
                correlatePairs(bandIndex, 4, 2, correlates, 0, 1);
                correlatePairs(bandIndex, 4, 2, correlates, 2, 3);
                correlatePairs(bandIndex, 2, 2, correlates, 0, 7);
                break;
            case CircularMicArray::FourPair:
                correlatePairs(bandIndex, 4, 1, correlates, 0, 4);
                break;
            case CircularMicArray::Orthogonal:
                correlatePairs( bandIndex, 4, 2, correlates, 0, 4 );
                break;
            case CircularMicArray::OrthogonalAndNeighbours:
                correlatePairs( bandIndex, 4, 2, correlates, 0, 4 );
                correlatePairs( bandIndex, 1, 2, correlates );
                break;
            case CircularMicArray::Neighbours:
                correlatePairs( bandIndex, 1, 1, correlates );
                break;
            case CircularMicArray::NextNeighbours:
                correlatePairs( bandIndex, 2, 1, correlates );
                break;
            case CircularMicArray::NextNextNeighbours:
                correlatePairs ( bandIndex, 3, 1, correlates );
                break;
            case CircularMicArray::Qua:
                correlatePairs ( bandIndex, 1, 2, correlates, 0, 4 );
                correlatePairs ( bandIndex, 2, 2, correlates, 3, 7 );
                correlatePairs ( bandIndex, 3, 2, correlates, 4, 7 );
                correlatePairs ( bandIndex, 4, 2, correlates, 3, 6 );
                break;
            case CircularMicArray::Half:
                correlatePairs ( bandIndex, 4, 2, correlates, 0, 4 );
                correlatePairs ( bandIndex, 3, 2, correlates );
                correlatePairs ( bandIndex, 2, 2, correlates );
                correlatePairs ( bandIndex, 1, 2, correlates );
                break;
            case CircularMicArray::All:
                correlatePairs ( bandIndex, 4, 2, correlates, 0, 4 );
                correlatePairs ( bandIndex, 3, 1, correlates );
                correlatePairs ( bandIndex, 2, 1, correlates );
                correlatePairs ( bandIndex, 1, 1, correlates );
                break;
            default:
                LogFmtA ( "E_ Unknown correlation strategy!" );
                throw new std::runtime_error ( "correlation strategy malcofigured!" );
            }

        }
        combination->combine( correlates );
        addResult(correlates.GetHead(),bandIndex,continued);
        correlates.DeleteAll();
    }
}

void PoAPLocalizerCircular::initBackprojection()
{
    LogMsgA("PoAPLocalizerCircular.initBackprojection");

    if (backProCirc!=0) {
        delete backProCirc;
        backProCirc=0;
        backProCirc = 0;
    }

    if (elevationMax<5.0) {
        backProCirc = new AzimuthCircleBackprojection(samplingFrequency, mic.getDiameter()*1e-2, mic.getMicrophoneCount(), azimuthStep);
        backProCirc->setExtraAngularOffset(rotation);
        backProCirc->setAlternatingHeight(mic.getAlternatingHeight());
        backProCirc = backProCirc;
        backProCirc->precalc();
        LogFmtA("OnlineLocalizer usign circular backProCircjection with %f steps.", azimuthStep);
    } else {

        backProCirc = new AzimuthElevationBackprojectionCircArray(samplingFrequency, mic.getDiameter()*1e-2, mic.getMicrophoneCount(), 1.5, elevationMax, azimuthStep, elevationStep);
        backProCirc->setExtraAngularOffset(rotation);
        backProCirc->setAlternatingHeight(mic.getAlternatingHeight());
        backProCirc = backProCirc;
        //backProCirc->setInterpolation(false);
        backProCirc->precalc();
        LogFmtA("OnlineLocalizer using spherical backProCircjection with %f / %f steps up to %f.", azimuthStep, elevationStep, elevationMax);
    }

    //    windowTime = (double)windowSize / (double)samplingFrequency;
    correlator.DeleteAll();
    //correlator.RemoveAll();
    sampleDistance = soundSpeed / samplingFrequency;
    //stepSamples = ceil( frameStep*1e-3*samplingFrequency );
    double frameSamples = correlationSizeMs * 1e-3 * samplingFrequency;
    int maxSamples = correlationStep-40;
    for (int band=0;band<filterbanks.GetHead()->getCount();band++) {
        double f = filterbanks.GetHead()->getFrequency(band);
        double w = Hearing::Gammatones::bandwidthGlasbergMoore(f);
        if (w > 1000.0) w = 1000.0;
        if (f<50.0) {
            f=50.0;
        }
        double flow = f - 2.0 * w;
        if (flow<50.0) {
            flow=50.0;
        }
        for (int offset=0;offset<4;offset++) {
            double distance = soundSpeed / f; // flow?
            double freqSamples = 2.0 * distance / sampleDistance;
            double baseSample = mic.getMicDistance(offset+1) * samplingFrequency / soundSpeed;
            int samples = ceil(freqSamples + baseSample + frameSamples);
            if (samples > maxSamples)
                samples = maxSamples;
       //   LogFmtA("prepare band %2d %.2fkHz, mic-offset %d:  %.1f +  %.1f + %.1f = %d samples / %.1fms", band, f*1e-3 ,offset, baseSample, frameSamples, freqSamples, samples, (double)samples*1000 / samplingFrequency);
            correlator.Insert( band*4 + offset,new Neuro::SpikeWavCorrelator(samples,windowType) );
        }
    }
}

