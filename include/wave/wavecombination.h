#ifndef WAVECOMBINATION_H
#define WAVECOMBINATION_H

#include "wave/wavgroup.h"
#include "wave/floatwave.h"

class WaveCombination
{
public:
    virtual QSharedPointer<Audio::Wave> combine(Audio::WavGroup &correlates,QSharedPointer< Audio::Wave > temp)=0;
    virtual void prepare(Audio::Wave &correlate) {}
};


class WaveGroupNormalization
{
public:
    virtual double getAbsMax(Audio::WavGroup &waves)
    {
        double mi=0.0,ma=0.0;

        long l = waves.first()->getFrameCount();
        long ch = waves.first()->getChannels();
        for (int j=0;j<waves.count();j++) {
            for (int c=0;c<ch;c++) {
                for (int i=0;i<l;i++) {
                    double v = waves.at(j)->getSample(i,c);
                    if (v>ma) ma=v;
                    if (v<mi) mi=v;
                }
            }
        }
        double f = std::max<double>(-mi,ma);
        return f;
}

    virtual double normalize(Audio::WavGroup &waves,double target=1.0)
    {
        double f = getAbsMax(waves);
        if (f==0.0) return 1.0;
        //if  (fabs(f-1.0)<1e-6) return 1.0;
        double _f=target/f;
        scale(waves,_f);
        return f;

    }

    virtual void scale(Audio::WavGroup &waves,double factor)
    {
        long l = waves.first()->getFrameCount();
        long ch = waves.first()->getChannels();
        for (int j=0;j<waves.count();j++) {
            for (int c=0;c<ch;c++) {
                for (int i=0;i<l;i++) {
                    waves.at(j)->setSample(i,c, factor*waves.at(j)->getSample(i,c));
                }
            }
        }
    }

};

class SumCombination : public WaveCombination
{
public:
    virtual QSharedPointer<Audio::Wave> combine(Audio::WavGroup &correlates,Audio::Wave* temp)
    {
        long l = correlates.first()->getFrameCount();
        long ch = correlates.first()->getChannels();
        for ( int c=0;c<ch;c++ ) {
            for (int i=0;i<l;i++) {
                double s =0;
                for (int j=0;j<correlates.count();j++) {
                    s += correlates.at(j)->getSample(i,c);
                }
                correlates.first()->setSample(i,c,s);
            }
        }
        //int nn = correlates.GetCount();
        //correlates.DeleteTail(nn-1);
        return correlates.first();
    }

};

class ProductCombination : public WaveCombination
{
public:
    virtual QSharedPointer<Audio::Wave> combine(Audio::WavGroup &correlates,QSharedPointer< Audio::Wave > temp);
    virtual void prepare(Audio::Wave &correlate);
};

class EnergyCombination : public WaveCombination
{
public:
    virtual QSharedPointer<Audio::Wave> combine(Audio::WavGroup &correlates,QSharedPointer< Audio::Wave >  temp);

};



#endif // WAVECOMBINATION_H
