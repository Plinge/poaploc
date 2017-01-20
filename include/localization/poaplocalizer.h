#ifndef POAPLOCALIZER_H
#define POAPLOCALIZER_H
#pragma once


#include "filter/azimuthelevationfilter.h"

#include "wave/wavgroup.h"
#include "neuro/spikecorrelator.h"
#include "wave/wavecombination.h"
#include "localizedspectrum.h"
#include "neuro/histogram.h"
#include "neuro/poapifier.h"


void dumpWave(const char* text, Audio::Wave*w,bool always=false);

class PoAPLocalizer : public PoAPifier
{
public:

    typedef enum {
        NORMAL = 0,
        NOCOR = 1
    } ProcessingMode;

    typedef enum {
        SUME = 0,
        MAXE = 1,
        KEEP = 2
    } ElevationMode;

protected:
    ProcessingMode processingMode;
    ElevationMode elevationMode;
    QMap<int, QSharedPointer< Neuro::SpikeWavCorrelator> > correlator;
    WaveCombination* combination;

    QList<QSharedPointer<WavGroupAzimuthElevation>> bandsList;
    QList<QSharedPointer<AzimuthElevationFilter>> filterList;
    // QVector<Audio::FloatWave> results[2];
    QList<QSharedPointer<Audio::WavGroup>> resultsSpectral[2];

    double resultTime[2];

    bool result_lk;
    double azimuthStep;
    double elevationStep, elevationMax, elevationMin;
    double sampleDistance;
    double soundSpeed;
    bool  backProInterpolated;

    int windowType;
    int correlationCount,windowStep;
    unsigned int correlationStep;    

    int postDeg1,postDeg2;
    int postEn,postBands;
    double postTime,postATh,postStep;
    bool postPeak;

    double spatialLimit;
    double correlationSizeMs;
    double rotation;

    bool doSumBands;

    int resultIndex;
    unsigned int historySize;
    double lastPostTime;

public:
    // ctor, dtor
    PoAPLocalizer();
    virtual ~PoAPLocalizer();

    // setter
    virtual void setProcessingMode(ProcessingMode m) {
         processingMode=m;
    }

    virtual void setElevationMode(int e);

    /**
     * setup backprojection.
     * @param azStep: azimuth step in degrees
     * @param rot: rotation offset in degrees, will rotate the azimuth by the given value
     * @param elevationMax: maximum elevation in degrees.
     *                      Hack! elevationMax <= -999 will turn off projection and
     *                      produce plain TDoA for debugging
     * @param elStep: elevation step in degrees
     * @param inter: use interpolation of TDoA values
     * @param elevationMin:  minimum elevation in degrees
     */
    virtual void setBackpro(double azStep,double rot,double elevationMax=0.0, double elStep=5.0,  bool inter=true, double elevationMin=0.0);

    virtual void setCorrelation(double frame,double spatial=99,int window=1);
    virtual void setPost(double t, int bands, double en=-120.0, int d1=5.0, int d2=45.0, double a=0,bool p=false,double s=0);
    virtual void setBandSum(bool b) { doSumBands = b; }
    virtual bool getBandSum() { return  doSumBands;  }
    virtual int  getInputFrameCount() const;

    virtual int getOutputBandCount() const;
    // calculation
    virtual void precalc();
    virtual void clear();

    virtual int getAzimuthBins()
    {
        AzimuthElevationBackprojection* p = getBackProjection();
        if (!p) return 0;
        return p->getAzimuthStepCount();
    }

    virtual int getElevationBins()
    {
        AzimuthElevationBackprojection* p = getBackProjection();
        if (!p) return 0;
        return p->getElevationStepCount();
    }

    virtual QVector<float> getSpikeCounts();

    virtual QVector<float> getSpikeSums();

    virtual QVector<float> getSpikeHistograms(int bins);

	virtual int getPostMinEn()
	{
		return postEn;
	}

    virtual int getOutputMicPairCount() const
    {
        return 1;
    }
protected:

    virtual void initBackprojection()=0;
    virtual AzimuthElevationBackprojection* getBackProjection()=0;
    virtual void correlate(bool continued)=0;
    virtual int  getPairHash(int index1,int index2,const int n);
    virtual void addResult(Audio::Wave * wave, int bandIndex, bool continued, int pairIndex=0);

    static short sumToEnergy ( double sum );
    virtual void calcSumAngleTime(Audio::Wave & out,int threshold,Audio::WavGroup & data);
            
    virtual bool updateSpectralResult();

    virtual int  getMaxSpikeIndex();

    void freeAll();

};

#endif // POAPLOCALIZER_H
