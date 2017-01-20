#ifndef POAPIFIER_H
#define POAPIFIER_H


#include "filter/auditoryfilterbankf.h"
#include "wave/floatwave.h"
#include "wave/sparsewave.h"
#include "filter/movingaverage.h"
#include "neuro/histogram.h"
#include "neuro/spikegeneratorpoapd.h"

#include <QVector>
#include <QSharedPointer>

class PoAPifier
{
public:
    typedef enum   {
        OFF = 0,
        BANDS = 1,
        EMPF = 2,
        EMPF_AUTO = 3,
        AUTO = 4
    } GainMode;
protected:
    QVector< QSharedPointer<AuditoryFilterbankF> > filterbanks;
    QVector< QSharedPointer<Audio::FloatWave> > averaged; //< overhang of average
    QVector< QSharedPointer<Audio::SparseWave> > spiked;
    QVector< QVector<float> > buffers;
    QVector< QSharedPointer<MovingAverage> > averages;
    QVector< QSharedPointer<SparseHistogram> > histograms;
    QVector< QSharedPointer<Neuro::SpikeGeneratorPoaPrecedence > > spikers;
    QVector<double> bandGains;

    int calcIteration;
    int debug;

    int poaDelay;
    int poaAverage;
    bool fastPoA;

    double absoluteSpikeThreshold;
    QVector< double > lastGain;
    double energyestimate;
    unsigned int samplingFrequency;

    GainMode gainMode;
    double gainOffset,gainMax,gainSmooth;
    double addGain;

    QVector< QVector<float> > inputData;
    unsigned int fftOverlap;
    int frameCount; //< actual size of a frame, should be windowSize * windowCount
    unsigned int bandCount;
    unsigned int channels;
    unsigned int pairCount;
    int correlationOverlap;
    double inputTime;
    double frameTime;
    int lastSampleCountFiltered;
    double histogramTime;

public:
    PoAPifier();
    virtual ~PoAPifier();

    virtual void setSamplingFrequency(int f);
    virtual double getSamplingFrequency();
    virtual int getChannelCount()=0;
    virtual void setGain(GainMode gm=OFF,double smooth=0,double ma=90);
    virtual void setFilter(int n,int f1,int fn,double gg=0,double scale = 1.0);
    virtual void setSpikes(double pre, double len, double gain=1.0, double th=9.0, double tha=0.0, int mode=1);

    virtual double getLastGain(int channel,int band);

    virtual double getActiveAverageGain()  {
        double sum=0;
        foreach ( float v , lastGain) {
            sum+=v;
        }

        return sum / (double)(lastGain.count());
    }

    virtual double getEnergyEstimate()
    {
        return energyestimate;
    }

    void precalc();

    void calc(const float* data, unsigned int frames,  bool continued);

    void setDebug(int on)  {
        debug=on;
    }

    virtual double getInputTime() const;
    virtual double getFrameLength() const;
    virtual int getBandCount() const;


    virtual QVector<double> getCenterFrequencies() const;

protected:

    virtual void createBuffers();

    virtual void updateGains();
    void setLastGain(int channel,int band,double g);
    void resetLastGain();
    virtual void updateGain(int bandIndex,int channel,double gain);

    virtual void process(int channel, int framePos, int frameChannels, int sampleCountFiltered, bool continued);

//    void processData(const short* data, int framePos, int frameChannels, int sampleCountFiltered, bool continued);
//    void processData(const long* data, int framePos, int frameChannels, int sampleCountFiltered, bool continued);
    void processData(const float* data, int framePos, int frameChannels, int sampleCountFiltered, bool continued);

    void freeAll();
};

#endif // POAPIFIER_H
