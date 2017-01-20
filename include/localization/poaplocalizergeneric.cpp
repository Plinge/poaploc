#include "localization/poaplocalizergeneric.h"
#include "localization/hamachercombination.h"
#include "wave/wavefile.h"

PoAPLocalizerGeneric::PoAPLocalizerGeneric()
    : backPro(0), azimuthMax(360.0), mic(0)
{

}

PoAPLocalizerGeneric::~PoAPLocalizerGeneric()
{
    if (backPro) {
        delete backPro;
    }
}

void PoAPLocalizerGeneric::setMic(MicArray* m)
{
    if (mic!=0) {
        delete mic;
    }
    mic = m;
    channels=mic->getMicrophoneCount();
    pairCount = 0;
    for (int index1=0;index1<channels-1;index1++) {
        for (int index2=index1+1;index2<channels;index2++) {
            pairCount++;
        }
    }
}

void PoAPLocalizerGeneric::setInputFrameCount(int n)
{
    frameCount = n;
}

int  PoAPLocalizerGeneric::getChannelCount()
{
    if (!mic) return 0;
    return mic->getMicrophoneCount();
}

AzimuthElevationBackprojection* PoAPLocalizerGeneric::getBackProjection()
{
    return backPro;
}

void PoAPLocalizerGeneric::setCombination(double g)
{
    if (combination!=0) {
        delete combination;
    }
    combination=0;
    if (g < 0.0) {
        return;
    }
    //combination.setGamma(g);
    if (g<=1.001) {
        combination  = new HamacherCombination(g);
    } else if (g<1.1) {
        combination =  new ProductCombination();
    } else {
        combination =  new EnergyCombination();
    }
}

void PoAPLocalizerGeneric::setCorrelationFraming(double len_ms,double step_ms)
{
    correlationSizeMs = len_ms;
    int correlationSize = samplingFrequency*len_ms*1e-3;
    if (step_ms > 0 ) {
        correlationStep = samplingFrequency*step_ms*1e-3;
    } else {
        correlationStep = -step_ms;
    }
    if (correlationStep<=0 || correlationSize<=0) {
        //fprintf(stderr,"FATAL cor sz = %d cor step = %d! frames = %d fs = %d len = %f step = %f\n", correlationSize, correlationStep, frameCount, samplingFrequency, len_ms, step_ms);
        throw std::runtime_error("PoAPLocalizerGeneric Parameter Error: Correlation step near zero!");
    } else {
        //correlationCount = 1+floor((frameCount-correlationSize) / correlationStep);
        correlationCount = frameCount / correlationStep;
//        int correlationOverlap = 0;
//        if (overlap) {
//            int correlationOverlapCount = correlationCount-(1+floor((frameCount-correlationSize) / correlationStep));
//            correlationOverlap = correlationOverlapCount * correlationStep;
//        }
        if (correlationCount<1) {
            std::cerr << "step " << correlationStep << "size " << correlationSize << std::endl;
            throw std::runtime_error("PoAPLocalizerGeneric Parameter Error: Correlation count zero!");
        }
    }
}

void PoAPLocalizerGeneric::precalc()
{
    PoAPLocalizer::precalc();
    channels = getChannelCount();
    bandsList.clear();
    filterList.clear();
    int pairsOut = getOutputMicPairCount();
    resultsSpectral[0].clear();
    resultsSpectral[1].clear();
    for (int pairIndex=0;pairIndex<pairsOut;pairIndex++) {
        WavGroupAzimuthElevation* bands = new WavGroupAzimuthElevation(getBackProjection());
        bands->resize(bandCount);
        for (int band=0;band<bandCount;band++) {
            bands->setAt(band,0);
        }
        AzimuthElevationFilter* filter = new AzimuthElevationFilter(bands, getBackProjection());
        bandsList.append(QSharedPointer<WavGroupAzimuthElevation>(bands));
        filterList.append(QSharedPointer<AzimuthElevationFilter>(filter));
        //results[0].append(FloatWave());
        //results[1].append(FloatWave());
        Audio::WavGroup* g= new Audio::WavGroup();
        resultsSpectral[0].append(QSharedPointer<Audio::WavGroup>(g));
        g= new Audio::WavGroup();
        resultsSpectral[1].append(QSharedPointer<Audio::WavGroup>(g));
    }

    calcIteration=0;
    nextCenterOffset=0;
    if (postTime>0.0) {
        historySize = ceil( (postTime*samplingFrequency) / correlationStep);
        if (historySize<correlationCount) {
//            LogFmtA("W_ History size %D < correlation count %D, frame count is to large for given post time!", historySize,  correlationCount);
            historySize = correlationCount;
        }
    } else {
        historySize = correlationCount;
    }
    createBuffers();

    resultIndex=0;
//    if (debug) {
//        if  (fastPoA) {
//            LogFmtA("PoAPLocalizerGeneric::precalc()  Using Fast PoA for Spikes.");
//        } else {
//            LogFmtA("PoAPLocalizerGeneric::precalc()  Using poa delay = %d for Spikes.", poaDelay);
//        }
//        LogFmtA("PoAPLocalizerGeneric::precalc() %d history, %d correlations per block, %d frames per block, %d fft  overlap, backprojection %s",
//                historySize , correlationCount,  frameCount,  fftOverlap,  (backProInterpolated ? "interpolated" : "rounded"));

//        spikers[0].log();
//    }


    resetLastGain();
    for (int bandIndex=0;bandIndex<bandCount;bandIndex++) {
        for (int channel=0;channel<channels;channel++)   {
            updateGain(bandIndex,channel,0.0);
        }
    }
}

void PoAPLocalizerGeneric::calc(const float* data, unsigned int frames,  bool continued)
{
    PoAPifier::calc(data,frames,continued);

    correlate(continued);

    if (processingMode != NOCOR) {
        if (postStep<=0 || (lastPostTime<inputTime-postStep)) {
            updateSpectralResult();
            lastPostTime=inputTime;
        }
    }
    calcIteration++;
}

void PoAPLocalizerGeneric::initCorrelation()
{
    correlator.clear();
    bandCount = filterbanks.first()->getCount();
    int micCount = mic->getMicrophoneCount();
    sampleDistance = soundSpeed / samplingFrequency;
    //stepSamples = ceil( frameStep*1e-3*samplingFrequency );
    double frameSamples = correlationSizeMs*1e-3 *samplingFrequency;
    // int maxSamples = correlationStep-40;
    int maxFrameSamples = 0;
    int minFrameSamples = samplingFrequency;
    for (int band=0;band<bandCount;band++) {
        double f = filterbanks.first()->getFrequency(band);
        double w = Hearing::Gammatones::bandwidthGlasbergMoore(f);
        if (w > 1000.0) w = 1000.0;
        if (f<50.0) {
            f=50.0;
        }
        double flow = f - 2.0 * w;
        if (flow<50.0) {
            flow=50.0;
        }
        double distance = soundSpeed / f; // flow?

        for (int index1=0;index1<micCount-1;index1++) {
            for (int index2=index1+1;index2<micCount;index2++) {
                int key = getCorrelatorIndex(band,index1,index2);
                double micDistance = mic->getMicPosition(index2).distance(mic->getMicPosition(index1));
                double freqSamples = 2.0 * distance / sampleDistance;
                double baseSample = fabs(micDistance) / sampleDistance;
                int samples = ceil(freqSamples + baseSample + frameSamples);
//                if (samples > maxSamples) {
//                    LogFmtA("Allowing only %D samples of %D", maxSamples, samples);
//                    samples = maxSamples;
//                }
                if (samples > maxFrameSamples) {
                    maxFrameSamples = samples;
                }
                if (samples < minFrameSamples) {
                    minFrameSamples = samples;
                }
//                if (index2==index1+1) {
//                    LogFmtA("prepare band %d %.2fkHz, mics %d,%d:  %.1f +  %.1f + %.1f = %d samples / %.1fms",
//                        band, f*1e-3 ,index1, index2, baseSample, frameSamples, freqSamples, samples, (double)samples*1000 / samplingFrequency);
//                }
                correlator.insert(key, QSharedPointer< Neuro::SpikeWavCorrelator > (
                                      new Neuro::SpikeWavCorrelator(samples,windowType)
                                 ));
            }
        }
    }
    correlationOverlap = maxFrameSamples;
}

void PoAPLocalizerGeneric::initBackprojection()
{
    if (backPro!=0) {
        delete backPro;
        backPro=0;
    }

    azimuthMax = 360.0;
//    if (mic->isLinear()) {
//        azimuthMax = 180;
//        std::cerr <<  "Microphone array is linear!" << std::endl;
//        //rotation = -90;
//        /// @FIXME: determine rotation from array orientation
//    }
    if  (debug) {
        std::cerr << "azimuth precision " << azimuthStep <<  " rotation " << rotation << " max " << azimuthMax << std::endl;
    }
    // // AzimuthElevationBackprojectionGen(double fs,MicArray* m,double dm=1.5,int ev=90,double as=1,double es=5,double am=360.0)
    if (elevationMax>-998) {
        backPro = new AzimuthElevationBackprojectionGen(samplingFrequency,mic,1.5,elevationMax,azimuthStep,elevationStep,azimuthMax,elevationMin);
        backPro->setVerbose(debug?1:0);
        backPro->setExtraAngularOffset(rotation);
        backPro->precalc();
        backPro->setInterpolation(backProInterpolated);
        // backPro->dump(1*8+2, 8);    exit(10);
    }
    initCorrelation();
}

int framesDone=0;

void PoAPLocalizerGeneric::correlate(bool continued)
{
    int channels = mic->getMicrophoneCount();
    bandCount = filterbanks.first()->getCount();

    updateGains();

    if (processingMode == NOCOR) {
        return;
    }

    firstCenter = correlationOverlap + nextCenterOffset; // - correlationStep;

    int correlationsPossible = (frameCount-(correlationOverlap>>1) - (firstCenter-correlationOverlap)) / correlationStep;
    correlationsPossible++;
    int nextCenter = firstCenter - correlationOverlap + correlationStep * (correlationsPossible);
    nextCenterOffset = nextCenter - frameCount;
//    LogFmtA("correlating over %5d .. %5d, %d correlation centers %5d .. %5d, overlap %d, next %d offset %d",
//            firstCenter-correlationOverlap/2,
//            firstCenter+correlationOverlap/2 + correlationStep * (correlationsPossible-1),
//            correlationsPossible,
//            firstCenter - correlationOverlap,
//            firstCenter - correlationOverlap + correlationStep * (correlationsPossible-1),
//            correlationOverlap,
//            nextCenter,
//            nextCenterOffset);

    nextCenterOffset = nextCenter - frameCount;
//    LogFmtA("absolute cor %d .. %d + %d",
//            framesDone + firstCenter - correlationOverlap,
//            framesDone + firstCenter - correlationOverlap + correlationStep * (correlationsPossible-1),
//            correlationStep
//           );


    framesDone+=frameCount;

#pragma omp parallel for
    for (int bandIndex=0;bandIndex<bandCount;bandIndex++) {
        int spikedIndex = channels*bandIndex;
        Audio::WavGroup correlates;
        for (int index1=0;index1<channels-1;index1++) {
            for (int index2=index1+1;index2<channels;index2++) {
                int key = getCorrelatorIndex(bandIndex,index1,index2);
                Neuro::SpikeWavCorrelator* cor = correlator[key].data();
                double micDistance = mic->getMicPosition(index2).distance(mic->getMicPosition(index1));
                int chs = 2*(ceil(micDistance/sampleDistance)+1);
                if (chs<=1) {
                    qWarning("Only %d correlation channels?!", chs);
                }
                if (!backPro) {
                    chs = 99;
                }
                Audio::FloatWave correlate;                
                cor->calcForChannels(chs, *spiked[index1 + spikedIndex], 0, *spiked[index2 + spikedIndex], 0, correlate, correlationStep,firstCenter,correlationsPossible);
                if (combination!=0) {
                    combination->prepare(correlate);
                }
//                if (debug) {
//                     std::cerr << "band " << bandIndex << " pair " << index1 << "," << index2 << " correlations "  << chs << std::endl;
//                     dumpWave("correaltion ", &correlate, true);
//                }
//                if (bandIndex == 0 && index1==1 && index2==4) {
//                    dumpWave("correaltion ", &correlate, true);
//                }
                if (debug && calcIteration>=debug && bandIndex==4 && index1==0 && index2==1) {
                    Audio::WaveFile w;
                    int n = correlate.getChannels();
                    w.create(correlate.getSamplingFrequency(),n,correlate.getFrameCount());
                    for (int i=0;i<n;i++) {
                        correlate.copyChannelTo(&w,i,-1,i);
                    }
                    w.write("PL_Correlate_4_0_1.wav");
                    dumpWave("correaltion ", &correlate, true);
                    std::cerr << "dumped correlation " << w.getNonzeroSampleCount() << "samples" << std::endl;
                }
                //if (debug) {
                //    int nz = correlate.getNozeroSampleCount();
                //    //int ts = correlate.getFrameCount();
                //    if (nz>0) {
                //        qDebug("Correlated %2d %2d Band %2d : got %9d samples (0<x<%.3f).",
                //            index1, index2, bandIndex,  nz, correlate.getMaxSample());
                //    }
                //}
                int hash = getPairHash(index1,index2,channels);
                Audio::FloatWave* p= new Audio::FloatWave();
//                qDebug("Using backpro %02d for mics %d,%d (%f,%f) (%f,%f) %.2fm",
//                        hash,index1,index2,
//                        mic->getMicPosition(index1).x, mic->getMicPosition(index1).y,
//                        mic->getMicPosition(index2).x, mic->getMicPosition(index2).y,
//                        mic->getMicDistance(index1,index2));
                if (debug && bandIndex == 4 && index1==1 && index2==1) {
                    dumpWave("correlated  ", &correlate);
                }
                if (backPro) {
                    backPro->calcFloat(hash, correlate, *p);
                } else {
                    p->create(correlate.getSamplingFrequency(),correlate.getChannels(),correlate.getFrameCount());
                    p->zero();
                    for (int sampleIndex=0;sampleIndex<correlate.getFrameCount();sampleIndex++) {
                        for (int ch=0;ch<correlate.getChannels();ch++) {
                           p->setSample(sampleIndex,ch,correlate.getSample(sampleIndex,ch));
                        }
                    }
                }
                correlates.append(QSharedPointer<Audio::Wave>(p));
//                cerr << "band " << bandIndex << " mics  "<<  index1 << ", " <<  index2 << "correlated samples = " << correlate.getFrameCount()<< ", " << p->getFrameCount() <<  endl;
                if (debug && bandIndex == 4 && index1==1 && index2==4) {
                    dumpWave("projected  ", p);
                }
                if (debug && calcIteration>=debug && bandIndex==4 && index1==0 && index2==1) {
                    Audio::WaveFile wp(*p);
                    wp.write("PL_Backpro_4_0_1.wav");
                }
            }
        }
        if (combination!=0) {
            QSharedPointer<Audio::Wave> temp = QSharedPointer< Audio::Wave > (new Audio::FloatWave());
            QSharedPointer<Audio::Wave> res = combination->combine(correlates, temp);
            if (debug && res) {
                int nz = res->getNonzeroSampleCount();
                if (nz>0) {
                    double pr = (1000.0 * nz) / ((double)res->getFrameCount() * (double)res->getChannels());
                    qDebug("Combined Band %2d %4.2fKHz: got %9d samples (0<x<%.3f) %4.1f/1000 in %dx%.1fms=%.3fs.",
                        bandIndex,  filterbanks[0]->getFrequency(bandIndex) *1e-3, nz, res->getMaxSample(), pr, res->getFrameCount(), 1000.0 / res->getSamplingFrequency(), correlates.first()->getTotalSeconds() );
                }
                if (bandIndex==0) {
                    QSharedPointer<Audio::Wave> p = correlates.first();
                    dumpWave("combined ", p.data(), true);
                }
            }
            addResult(correlates.first().data(), bandIndex, continued);
            correlates.clear();
        } else {
            int pairIndex=0;
            foreach(QSharedPointer<Audio::Wave> wp, correlates) {
                addResult(wp.data(), bandIndex, continued, pairIndex++);
            }
            correlates.clear();
        }
    }
}

double PoAPLocalizerGeneric::getMaxOutTime()
{	
	// case 1: using post average
	if (postTime > 0.0) {
		return inputTime - 0.5*postTime;
	}
	return inputTime;	
}

bool PoAPLocalizerGeneric::getLocalizedSpectra(std::list<LocalizedSpectrum> & res)
{
    if (lastPostTime<inputTime) {
        // no new results according to post step
        return false;
    }
    if (result_lk) return false;
  //  if (resultsSpectral[resultIndex].first().data().count()<1) return false;
    result_lk=true;
    res.clear();
    int pairIndex=0;
    foreach (QSharedPointer<Audio::WavGroup> resultSpectralPtr, resultsSpectral[resultIndex]) {

//    LogFmtA("PoAPLocalizerGeneric::getLocalizedSpectra() t = %.3f fs = %f n = %d == %fs.",
//                inputTime,
//                resultSpectral[resultIndex].GetHead()->getSamplingFrequency(),
//                resultSpectral[resultIndex].GetHead()->getFrameCount(),
//                resultSpectral[resultIndex].GetHead()->getTotalSeconds()
//                );
        Audio::WavGroup* result = resultSpectralPtr.data();
        if (result->count()<1) {
            continue;
        }
        int samples = result->first()->getFrameCount();
        if  (samples<1) {
            continue;
        }
        double sampleTime =  (double)correlationStep  / (double)samplingFrequency;
        double timeOffset = 0.0;
        if (postTime>0.0) {
            timeOffset = -0.5 * postTime;
        }
        // 1.0 / resultSpectral[resultIndex].GetHead()->getSamplingFrequency();
        //double totalTime = sampleTime*(samples-0.5);
        //double totalTime = resultSpectral[resultIndex].GetHead()->getTotalSeconds();
        int channels = result->first()->getChannels();

        double overlapTime = (double)(firstCenter-correlationOverlap) / (double)samplingFrequency;
        if (backPro==0) {
            for (int sampleIndex=0;sampleIndex<samples;sampleIndex++) {
                double time = inputTime + timeOffset + overlapTime + sampleIndex*sampleTime;
                if (time>0.0) {
                    for (int ch=0;ch<channels;ch++) {                        
                        std::vector<double> spectrum;
                        double spectralSum=0.0;
                        int count=0;
                        for (int band=0;band<result->getCount();band++) {
                            Audio::Wave* w = result->at(band).data();
                            double value = w ? w->getSample(sampleIndex,ch) : 0.0;
                            spectralSum+=value;
                            spectrum.push_back(value);
                            if (value>0.0) count++;
                        }
                        if ((doSumBands || count>postBands) && spectralSum>0.0) {
                            double spectralEnergy = 20.0*log10(spectralSum);
                            if (spectralEnergy>postEn) {
                                res.push_back(LocalizedSpectrum(time, ch, spectrum, 0, pairIndex));
                            }
                        }
                    }
                }
            }
        } else {
            int azimuthBins = getAzimuthBins();
            if (azimuthBins < 1) {
                qWarning("PoAPLocalizerGeneric %d azimuth bins?!",azimuthBins);
                result_lk=false;
                return true;
            }

            for (int sampleIndex=0;sampleIndex<samples;sampleIndex++) {
                double time = inputTime + timeOffset + overlapTime + sampleIndex*sampleTime;
                if (time>0.0) {
                    for (int ch=0;ch<channels;ch++) {
                        double angle = backPro->azimuth( ch % azimuthBins );
                        double elevation = backPro->elevation( ch / azimuthBins );
                        std::vector<double> spectrum;
                        double spectralSum=0.0;
                        int count=0;
                        for (int band=0;band<result->getCount();band++) {
                            Audio::Wave* w = result->at(band).data();
                            double value = 0.0;
                            if ((w) && (sampleIndex<w->getFrameCount()))
                                value = w->getSample(sampleIndex,ch);
                            spectralSum+=value;
                            spectrum.push_back(value);
                            if (value>0.0) count++;
                        }
                        if ((doSumBands || count>postBands) && spectralSum>0.0) {
                            double spectralEnergy = 20.0*log10(spectralSum);
                            if (spectralEnergy>postEn) {
                                res.push_back(LocalizedSpectrum(time, angle, spectrum, elevation, pairIndex));
                            }
                        }
                    }
                }
            }
        }
        ++pairIndex;
    }
	if (debug) {
		QString debug;
		for (std::list<LocalizedSpectrum>::iterator it=res.begin(); it != res.end(); ++it) { 
			QString tmp = QString("%1s %2,%3(%4)").arg(it->getTime(),0,'f',1).arg(it->getAngle(),0,'f',2).arg(it->getElevation(),0,'f',1).arg(it->getSpectralSum(),0,'f',1);			 
			if  (!debug.isEmpty()) debug.append(';');
			debug.append(tmp);
		}
		qDebug(debug.toLocal8Bit());
	}
    result_lk=false;
    return res.size()>0;
}
