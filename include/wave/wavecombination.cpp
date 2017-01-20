#include "wavecombination.h"
#include <assert.h>

void ProductCombination::prepare(Audio::Wave &correlate)
{

}

QSharedPointer<Audio::Wave> ProductCombination::combine(Audio::WavGroup &correlates, QSharedPointer<Audio::Wave> temp)
{

    int nn = correlates.count();
    QSharedPointer<Audio::Wave> head = correlates.first();
    long l = head->getFrameCount();
    for (int j=1;j<nn;j++) {
        long l2 =  correlates.at(j)->getFrameCount();
        if (l2!=l) {
           // LogFmtA("W_ Unequal sample count in ProductCombination::combine, %d %d!",l,l2);
            if (l2<l) l=l2;
        }
    }
    bool allfloat = true;
    for (int j=0;j<nn;j++) {
        if (correlates.at(j).data()->getTypeNumber()!=WAVE_TYPE_FLOATWAVE) {
            allfloat=false;
            break;
        }
    }
    long ch = head->getChannels();
    if (allfloat) {
        Audio::FloatWave* headf = (Audio::FloatWave*)correlates.first().data();
        long stride = l*ch;
        float* data = headf->getPtr();
        for (int j=1;j<nn;j++) {
            Audio::FloatWave* otherWave = (Audio::FloatWave*)correlates.at(j).data();
            float* other = otherWave->getPtr();
            // lets hope this is vectorized
            for (int i=0;i<stride;i++) {
                data[i]*=other[i];
            }
        }
    } else {
        for (int i=0;i<l;i++) {
            for ( int c=0;c<ch;c++ ) {
                double p =1.0;
                for (int j=0;j<nn;j++) {
                    double v = correlates.at(j)->getSample(i,c);
                    if (v==0.0) {
                        p=0; break;
                    }
                    p *= v;

                }
                head->setSample(i,c,p);
            }
        }
    }
    head->setSampleCount(l);
    return head;

}

QSharedPointer< Audio::Wave > EnergyCombination::combine(Audio::WavGroup &correlates, QSharedPointer<Audio::Wave> temp)
{
    int nn = correlates.count();
    Audio::Wave* head = correlates.first().data();
    long l = head->getFrameCount();
    for (int j=1;j<nn;j++) {
        long l2 =  correlates.at(j)->getFrameCount();
        if (l2!=l) {
           // LogFmtA("W_ Unequal sample count in EnergyCombination::combine, %d %d!",l,l2);
            if (l2<l) l=l2;
        }
    }
    long ch = head->getChannels();
    for (int i=0;i<l;i++) {
        for (int c=0;c<ch;c++) {
            double s = 0;
            bool zero = false;
            for (int j=0;j<nn;j++) {
                double v = correlates.at(j)->getSample(i,c);
                if (v>0.0) {
                    s += log(v);  // speech energy
                } else {
                    zero = true;
                    break;
                }
            }
            if (zero) {
                head->setSample(i,c,0.0);
            } else {
                head->setSample(i,c,exp(s));
            }
        }
    }
    head->setSampleCount(l);
    return correlates.first();
}
