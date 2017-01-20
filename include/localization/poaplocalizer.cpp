#include "localization/poaplocalizer.h"
#include "wave/wavefile.h"
#include <iostream>

PoAPLocalizer::PoAPLocalizer() : processingMode(NORMAL), elevationMode(SUME),
    azimuthStep(3.0), elevationStep(12.0), elevationMax(36.0), elevationMin(0.0),
    soundSpeed(343.0),
    correlationStep(1024), correlationCount(1), rotation(180.0),
    postPeak(false), correlationSizeMs(12), historySize(64), windowType(1), combination(0), doSumBands(false),
    result_lk(false)
{
    postTime=0; postStep=0; lastPostTime=0;
    gainMax=80;gainSmooth=0;
}

PoAPLocalizer::~PoAPLocalizer()
{
    freeAll();
    if (combination!=0) {
        delete combination;
    }
    combination=0;
}

 int  PoAPLocalizer::getInputFrameCount() const
{
    //return correlationCount*correlationStep;
    return frameCount;
}

int PoAPLocalizer::getOutputBandCount() const
{
    if (doSumBands && postPeak ==  false) return 1;
    return getBandCount();
}

void PoAPLocalizer::setElevationMode(int e)
{
    elevationMode = (ElevationMode)e;
}

void PoAPLocalizer::setCorrelation(double frame,double spatial,int t)
{
    correlationSizeMs = frame;
    spatialLimit = spatial;
    windowType = t;
}

void PoAPLocalizer::addResult(Audio::Wave * wave, int bandIndex, bool continued, int pairIndex)
{
    //int resIndex = bandIndex + pairIndex*bandCount;
    Audio::WavGroup* bands = bandsList.at(pairIndex).data();
    if (bands->count()<=bandIndex) {
        throw std::runtime_error ( "data malcofigured!" );
    }
    Audio::SparseWave* res = (Audio::SparseWave*)bands->at(bandIndex).data();

    // current result buffer will be shifted to make room for the number of
    // new correlations. if none are given, it is shifted by the default.
    int correlations = correlationCount;
    if (wave!=0) {
        correlations = wave->getFrameCount();
    }
    if (res == 0) {
        if (wave!=0) {
            res = new Audio::SparseWave(wave->getSamplingFrequency(), wave->getChannels(), historySize);
            bands->setAt(bandIndex,res);
//            LogFmtA("New band(%d) fs = %f len = %f", bandIndex, bands->GetAt(bandIndex)->getSamplingFrequency(),bands->GetAt(bandIndex)->getTotalSeconds() );
        }
    } else {
        if (!continued) {
            res->zero();
        } else {
            res->shiftBy(correlations);
        }
    }
    // fill up the last correlations with the current data
    if (wave!=0  && res != 0) {
        int of = historySize - correlations;
        if (of<0) {
            of=0;
            //qDebug() << "Correlation count " << correlationCount << "higher than history size" << historySize << "!";
        }
        for (int c = 0; c<wave->getChannels(); c++) {
            int w=0;//std::max<int>(0, correlations-correlationCount);
            for (;w<correlations;w++) {                
                res->setSample(of+w, c, wave->getSample(w,c));
            }
            if (w+of<historySize && debug) {
                std::cerr  << "band " << bandIndex << " only "  << correlations << "/" << correlationCount << " filling with zeros!"  << std::endl;
            }
            for (;w+of<historySize;w++) {
                res->setSample(of+w, c, 0.0);
            }
        }

//       if (bandIndex==0){
//#pragma omp critical
//            {
//               LogFmtA("Time %.3f  got %d  new correlations", inputTime, correlations);
//            }
//        }
    }
}

void PoAPLocalizer::setBackpro(double s,double r,double em, double es,bool inter,double e0)
{
    azimuthStep=s;
    rotation=r;
    elevationMax=em;
    elevationStep=es;
    elevationMin=e0;
    backProInterpolated=inter;
}

void PoAPLocalizer::setPost(double t,int bands,double en,int d1,int d2,double a,bool p,double s)
{
    postDeg1=d1;postDeg2=d2;
    postEn=en;postBands=bands;
    postTime=t;
    postATh=a;
    postPeak=p;
    if (s<=0.0 && postTime>0) {
        postStep=0.25*postTime;
    } else {
        postStep=s;
    }
}

void PoAPLocalizer::precalc()
{
    PoAPifier::precalc();
    initBackprojection();

}



void PoAPLocalizer::freeAll()
{
    PoAPifier::freeAll();

    correlator.clear();
    filterList.clear();
    bandsList.clear();
}

void PoAPLocalizer::clear()
{
    freeAll();
}

bool PoAPLocalizer::updateSpectralResult()
{

    if (debug) {
        AzimuthElevationFilter* filter = filterList.first().data();
        qDebug("PoAPLocalizer::updateSpectralResult() fs = %f n = %d == %fs.",
                filter->getResult(false)->first()->getSamplingFrequency(),
                filter->getResult(false)->first()->getFrameCount(),
                filter->getResult(false)->first()->getTotalSeconds()
                );
		dumpWave("update = ", filter->getResult(false)->first().data() );
    }
    
    int nextResultIndex = resultIndex^1;

    if (result_lk) {
        //LogFmtA("E_ PoAPLocalizer.calc skipping update, locked!");
         qWarning("PoAPLocalizer.calc skipping update, locked!");
         return false;
    }

    result_lk=true;

    QList<QSharedPointer<Audio::WavGroup>>::iterator it = resultsSpectral[nextResultIndex].begin();
    foreach(QSharedPointer<AzimuthElevationFilter> filterptr, filterList) {
        AzimuthElevationFilter* filter =filterptr.data();
        filter->clear();
        Audio::WavGroup* results = it->data();
        ++it;
        if (debug && frameCount >= debug) {
            QSharedPointer<Audio::Wave> p = filter->getResult(false)->first();
            Audio::WaveFile w(p.data());
            w.write("PL_Result_0.wav");
        }

        if (getBackProjection()) {
            if (getBackProjection()->getElevationStepCount() > 1 && !postPeak && elevationMode != KEEP) {
                if (elevationMode == MAXE) {
                    filter->postFilter(AzimuthElevationFilter::POST_MAXE,0,0,0);
                } else {
                    filter->postFilter(AzimuthElevationFilter::POST_SUME,0,0,0);
                }
                if (debug && frameCount >= debug) {
                    QSharedPointer<Audio::Wave > p = filter->getResult(true)->first();
                    Audio::WaveFile w(p.data());
                    w.write("PL_SumElevation_0.wav");
                }
            }
        }

        if (postTime>0.0) {
            if (debug) {
                qDebug("PoAPLocalizer post average over %d = %fs (post time = %fs)",
                    filter->getResult(true)->first()->getFrameCount(),
                    filter->getResult(true)->first()->getTotalSeconds(),
                    postTime);
            }
            filter->postAverage();    
            if (debug && frameCount >= debug) {
                QSharedPointer<Audio::Wave > p = filter->getResult(true)->first();
                Audio::WaveFile w(p.data());
                w.write("PL_Average_0.wav");
            }
        }

        // quirk: keep band information
        Audio::WavGroup preBandSum;
        if (getBackProjection()) {
            if (doSumBands) {
                Audio::WavGroup* r = filter->getResult(true);
                preBandSum.clone(*r);
        //        LogFmtA("Summing bands n>%d e>%f", postBands, (double)postEn);
                filter->postBandSum(postBands, postEn);
                if (debug && frameCount >= debug) {
                    QSharedPointer<Audio::Wave > p = filter->getResult(true)->first();
                    Audio::WaveFile w(p.data());
                    w.write("PL_BandSum_0.wav");
                }
            } else {
                //filter->postFilter(AzimuthElevationFilter::POST_THRES_BANDC,postBands, postEn,0);
                filter->postFilter(AzimuthElevationFilter::POST_THRESHOLD,32767.0* pow(2.0, (double)postEn/6.0),0,0);
            }

            if (postATh>=0.0) {
                if (debug) {
                    dumpWave("Average = ", filter->getResult(true)->first().data() );
                    qDebug("PoAPLocalizer.calc %s %d %d %f.",postPeak ? "grid" : "poa", postDeg1, postDeg2, postATh);
                }
                filter->postFilter(postPeak ? AzimuthElevationFilter::POST_AEGRID : AzimuthElevationFilter::POST_AZPOA, postDeg1, postDeg2, postATh);
                if (debug) {
                    dumpWave("PoA(P) = ",  filter->getResult(true)->first().data() );
                }
                if (debug && frameCount >= debug) {
                    QSharedPointer<Audio::Wave > p = filter->getResult(true)->first();
                    Audio::WaveFile w(p.data());
                    w.write("PL_Post_0.wav");
                }
            }

        }

        results->clear();
        Audio::WavGroup* r = filter->getResult(true);
        if (doSumBands && postPeak) {
//            qDebug("%d nonzero samples in result", r->first()->getNonzeroSampleCount());
//            results->clone(preBandSum,WAVE_TYPE_SPARSEWAVE);
//            for (int channel=0;channel<preBandSum.getChannels();channel++) {
//                for (int sample=0;sample<preBandSum.getFrameCount();sample++) {
//                    double v = r->first()->getSample(sample, channel);
//                    if (v<1e-9) {
//                        for (int band=0;band<preBandSum.getCount();band++) {
//                            results->at(band)->setSample(sample,channel,0.0);
//                        }
//                    }
//                }
//            }
            results->cloneZero(preBandSum,WAVE_TYPE_SPARSEWAVE);
            for (int channel=0;channel<r->getChannels();channel++) {
                int sample=-1,hint=-1;
                double v=0;
                while ((sample=r->first()->getNextNonzeroIndex(sample,channel,hint,v))>=0) {
                    for (int band=0;band<results->getCount();band++) {
                        double w= preBandSum.at(band)->getSample(sample,channel);
                        results->at(band)->setSample(sample,channel, w);
                    }
                }
            }
//            for (int band=0;band<results->getCount();band++) {
//                qDebug("%d nonzero samples in result, band #%d", results->at(band)->getNonzeroSampleCount(),band);
//            }
        } else {
            results->clone(*r);
        }
    }

    resultTime[nextResultIndex] = inputTime;
    resultIndex = nextResultIndex;
    result_lk=false;
    return true;
}

short PoAPLocalizer::sumToEnergy(double sum)
{
    sum = sqrt(sum);
    double en = (90.0+20.0*log10(sum))/90.0;
    if (en<0.0) en=0.0;
    return clip15s(en*32767.0);
}

void PoAPLocalizer::calcSumAngleTime(Audio::Wave &out, int threshold, Audio:: WavGroup& data)
{
    int bandCount = data.count();
    if (bandCount<1) return;
    int azimuthCount   = getBackProjection()->getAzimuthStepCount(); // 360!
    int elevationCount = data.getChannels() / azimuthCount; //backPro->getDim2Size();
    int sampleCount = data.getFrameCount();
    Audio::Wave* w =  data.first().data();
    if (w==0) return;
    int fout = w->getSamplingFrequency();    
    out.create(fout,azimuthCount+1 , ( azimuthCount+1 ) *sampleCount*2);
    double mv = threshold/32767.0;
#pragma omp parallel for
    for ( int azimuth=0;azimuth<azimuthCount;azimuth++ ) {
        int azimuthOffset = elevationCount*azimuth;
        for ( int sample=0;sample<sampleCount;sample++ ) {
            int count=0;
            double sum=0.0;
            for ( int band=0;band<bandCount;band++ ) {
                Audio::Wave* bw = data.at ( band ).data();
                if ( bw!=0 && bw->getFrameCount() > sample ) {
                    for ( int elevation=0;elevation<elevationCount;elevation++ ) {
                        double v = bw->getSample ( sample,azimuthOffset+elevation );
                        if ( v>mv ) {
                            sum +=  v;
                            count++;
                        }
                    }
                }
            }
            double en = sumToEnergy ( sum / ( elevationCount*bandCount ) );
            if (count >= postBands &&  en > postEn) {
                out.setSample( sample, azimuth+1, en );
            }
        }
    }
    for (int sample=0;sample<sampleCount;sample++) {
        out.setSample( sample,0,out.getSample ( sample,azimuthCount ) );
    }
}


void dumpWave(const char* text, Audio::Wave*w,bool always)
{
    if (!w) {
        std::cerr << text;
        std::cerr << "NULL";
        std::cerr << std::endl;
        return;
    }
    int debug=0;
    int  h=-1,p=-1,c=0;
    double v;
    while (debug<5) {
        p = w->getNextNonzeroIndexChannel(p,c,h,v);
        if  (p<0) {
            break;
        }
        if (!debug) {
            std::cerr << text;
        } else {
            std::cerr << ", ";
        }       
        std::cerr << v << "@" << p << ";" << c;
        debug++;
    }
    if (debug) {
        std::cerr << std::endl;
    } else if (always) {
        std::cerr << text << "empty" << std::endl;
    }
}

int PoAPLocalizer::getPairHash(int index1,int index2,const int n)
{
    return index1*n+index2;
}

/// new ideas

int PoAPLocalizer::getMaxSpikeIndex()
{
    double micSum[128];
    double micMax=0.0;
    int micMaxIndex=0;
    // int channels=getChannelCount();
    for (int micIndex=0;micIndex<channels;micIndex++) {
        micSum[micIndex]=0.0F;
        for (int band=0;band<bandCount;band++) {
            int spikedIndex = channels*band+micIndex;
            micSum[micIndex] += spiked[spikedIndex]->getSampleSum();
        }
        if  (micSum[micIndex]>micMax) {
            micMax=micSum[micIndex];
            micMaxIndex=micIndex;
        }
    }
    return micMaxIndex;
}

QVector<float> PoAPLocalizer::getSpikeCounts()
{
     QVector<float> data(channels*bandCount);
    int micMaxIndex = getMaxSpikeIndex();
    int  firstIndex = (micMaxIndex + channels - (channels>>1)) % channels;

    int dataIndex=0;
    for (int channel=0;channel<channels;channel++) {
        for (int band=0;band<bandCount;band++) {
            int spikeIndex = channels*band + ((channel +  firstIndex) %  channels);
            data[dataIndex++] = spiked[spikeIndex]->getNonzeroSampleCount();
        }
    }

    return data;
}

 QVector<float> PoAPLocalizer::getSpikeSums()
{
    QVector<float> data(channels*bandCount);
    int micMaxIndex = getMaxSpikeIndex();
    int  firstIndex = (micMaxIndex + channels - (channels>>1)) % channels;
    int dataIndex=0;
    for (int channel=0;channel<channels;channel++) {
        for (int band=0;band<bandCount;band++) {
            int spikeIndex = channels*band + ((channel +  firstIndex) %  channels);
            data[dataIndex++] = spiked[spikeIndex]->getSampleSum();
        }
    }
    return data;
}

QVector<float> PoAPLocalizer::getSpikeHistograms(int bins)
{
    QVector<float> data(channels*bandCount*bins);
    int micMaxIndex = getMaxSpikeIndex();
    int  firstIndex = (micMaxIndex + channels - (channels>>1)) % channels;
    int dataIndex=0;
    for (int channel=0;channel<channels;channel++) {
        for (int band=0;band<bandCount;band++) {
            int spikeIndex = channels*band + ((channel +  firstIndex) %  channels);
            Histogram hist(0.0, 1.0, bins, frameCount);
            hist.reset();
            //int samples = spiked[spikeIndex].getNozeroSampleCount();
            int i=0,h=0;
            double v;
            do {
                 i = spiked[spikeIndex]->getNextNonzeroIndex(i,0,h,v);
                 if (i>=0) {
                     hist.update(v);
                 } else {
                     break;
                 }
            } while (true);

            for (int bin=0;bin<bins;bin++) {
                data[dataIndex++] = hist.getCount(bin);
            }
        }
    }
    return data;
}

