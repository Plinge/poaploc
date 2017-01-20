#include "localization/hamachercombination.h"

void HamacherCombination::prepare(Audio::Wave &correlate)
{
    correlate.clip(0.0, 1.0);
}

bool HamacherCombination::combineRacing(Audio::Wave& in,Audio::Wave & add,Audio::Wave& out)
{
    long l1 = in.getFrameCount();
    long l2 = add.getFrameCount();
    l1=std::max<long>(l1,l2);
    if (out.getFrameCount() != l1 || out.getChannels() != in.getChannels()) {
        out.create(in.getSamplingFrequency(), in.getChannels(), l1);
    } else {
        out.zero();
    }
    //if (in.getDataSamplesTotal()==0 || add.getDataSamplesTotal()==0) return;
    long ch = in.getChannels();

//#pragma omp parallel for
    for ( int c=0;c<ch;c++ ) {
        int pos1=-1;
        int pos2=-1;
        int in1,in2;
        double v1,v2;
        in1 = in.getNextNonzeroIndex(-1,c,pos1,v1);
        in2 = add.getNextNonzeroIndex(-1,c,pos2,v2);
        do {
          if (in1<0) break;
          if (in2<0) break;
          while (in1<in2 && in1>=0) {
              in1 = in.getNextNonzeroIndex(in1,c,pos1,v1);
          }
          if (in1<0) break;
          if (in2<in1) {
            in2 = add.getNextNonzeroIndex(in1-1,c,pos2,v2);
          }
          if (in2<0) break;
          if (in1==in2) {
             out.setSample(in1, c, hamacherProduct ( v1 , v2 ) );
             in1 = in.getNextNonzeroIndex(in1,c,pos1,v1);
            // in2 = in1-1;
             in2 = add.getNextNonzeroIndex(in2,c,pos2,v2);
          }

        } while (in1>=0 && in2>=0);
    }
    return true;
}

bool HamacherCombination::combinePlain(Audio::Wave& in,Audio::Wave & add,Audio::Wave& out)
{
    long l1 = in.getFrameCount();
    long l2 = add.getFrameCount();
    long lout = std::max<long>(l1,l2);
    long lin  = std::min<long>(l1,l2);
    if (out.getFrameCount() != lout || out.getChannels() != in.getChannels()) {
        out.create(in.getSamplingFrequency(), in.getChannels(), lout, true);
    }

    long ch = in.getChannels();
    if (add.getChannels() != ch) {
        throw std::runtime_error("channel count mismatch in combination!");
        return false;
    }

    if (in.getTypeNumber()==add.getTypeNumber() && out.getTypeNumber()==in.getTypeNumber() && in.getTypeNumber()==WAVE_TYPE_FLOATWAVE) {
        int n = lin*ch;
        float* inp = ((Audio::FloatWave*)&in)->getPtr();
        float* adp = ((Audio::FloatWave*)&add)->getPtr();
        float* oup = ((Audio::FloatWave*)&out)->getPtr();
        while (n-->0) {
            *oup++ = hamacherProduct(*inp++,*adp++);
        }  
        // inputs are of different size, keep extras
        if (lin<lout) {
            n = (lout-lin)*ch;
            if (l1>l2) {
                while (n-->0) {
                  *oup++ = hamacherProduct(*inp,*inp++);
                }
            } else {
                while (n-->0) {
                  *oup++ = hamacherProduct(*adp,*adp++);
                }
            }           
        }
        return true;
    }

    for (int i=0;i<lin;i++) {
        for (int c=0;c<ch;c++) {
            double a=in.getSample(i,c);
            double b=add.getSample(i,c);
            if (a > 0.0 && b > 0.0) {
                out.setSample(i,c,hamacherProduct(a,b));
            }
        }
    }
    // inputs are of different size, keep extras
    if (lin<lout) {
        if (l1>l2) {
            for (int i=lin;i<lout;i++) {
                for (int c=0;c<ch;c++) {
                    double a=in.getSample(i,c);
                    out.setSample(i,c,hamacherProduct(a,a));
                }
            }
        } else {
            for (int i=lin;i<lout;i++) {
                for (int c=0;c<ch;c++) {
                    double a=add.getSample(i,c);
                  out.setSample(i,c,hamacherProduct(a,a));
                }
            }
        }
    }

    return true;
}


QSharedPointer<Audio::Wave> HamacherCombination::combine(Audio::WavGroup &correlates, QSharedPointer<Audio::Wave> temp)
{
    if (correlates.count() < 2) {
        return QSharedPointer<Audio::Wave>();
    }
    Audio::WavGroup pool;
    Audio::WavGroup correlates2;
    Audio::Wave* a = correlates.first().data();
    temp->create(a->getSamplingFrequency(),a->getChannels(),a->getFrameCount());
    pool.append(QSharedPointer<Audio::Wave>(temp));
    bool plain = correlates.first()->getFrameCount()<50;
    while (correlates.count()>1) {
        int n = correlates.count();
        for (int index=0;index<n;index+=2) {
            if (index+1<n) {
                //LogFmtA("HamacherCombination(%f) %d %d", gamma,index,index+1);
                QSharedPointer<Audio::Wave> p=pool.first(); pool.removeFirst();
                if (p==0) {
                    throw std::runtime_error("Empty pool in Hamacher Combination!");
                }
                QSharedPointer<Audio::Wave> a=correlates.at(index);
                QSharedPointer<Audio::Wave> b=correlates.at(index+1);
                if (!plain)  {
                    combineRacing(*a, *b, *p);
                } else {
                    combinePlain( *a, *b, *p);
                }
                correlates2.append(p);
                pool.append(a);
                pool.append(b);
            } else {
                //LogFmtA("HamacherCombination(%f) %d", gamma,index);
                QSharedPointer<Audio::Wave> p=pool.first(); pool.removeFirst();
                QSharedPointer<Audio::Wave> a=correlates.at(index);
                if (!plain) {
                    combineRacing(*a, *a, *p );
                } else {
                    combinePlain( *a, *a, *p );
                }
                correlates2.append(p);
                pool.append(a);
            }
        }
        correlates.discard();
        correlates += correlates2;
        correlates2.discard();
        // LogFmtA("HamacherCombination(%f) =>  %d left (pool %d)", gamma, correlates.GetCount(), pool.GetCount());
    }    
    pool.clear();
    return correlates.first();
}
