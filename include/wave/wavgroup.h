#pragma once
#include "wave/wave.h"
#include "wave/floatwave.h"
#include "wave/average.h"
#include <QVector>
#include <QSharedPointer>

namespace Audio {

/**
 * 
 * a group of waves representing a number of parallel channels
 *
 * @author axel plinge
 *
 * @par history
 *
 * - 2010-04-17  axel created
 */
class WavGroup : public  QVector< QSharedPointer< Wave > >
{
public:
    WavGroup() :   QVector< QSharedPointer< Wave > >()
	{

	}

	virtual ~WavGroup()
	{
        clear();
	}    

    void discard()
    {
        resize(0);
    }

	void clone(QVector<QSharedPointer<Wave> > &other,int typehint = WAVE_TYPE_FLOATWAVE );
    void cloneZero(QVector<QSharedPointer<Wave> > &other,int typehint = WAVE_TYPE_FLOATWAVE );

//	const AP_ArrayPtr<Wave> & operator = (const AP_ArrayPtr<Wave> & other)
//	{
//		CopyPtrs(other);
//		return *this;
//	}

    void setAt(int i,Wave*p) {
       (*this)[i]=QSharedPointer< Wave >(p);
    }

    void setAt(int i,QSharedPointer< Wave > p) {
       (*this)[i]=p;
    }

	/** sum */
	virtual void getSum (  Wave & sum ) 
	{
        Wave* head = first().data();
        sum.create( head->getSamplingFrequency() , head->getChannels() , head->getFrameCount(), true);
		head->copySamplesTo(&sum);
        int bandCount = count();
		double f = 1.0 / bandCount;
		sum.multiplyBy(f);
        int sampleCount= sum.getFrameCount();
		int channelCount =  sum.getChannels();
#pragma omp parallel for
		for ( int sample=0;sample<sampleCount;sample++ ) {
			for ( int channel=0;channel<channelCount;channel++ ) {
				double v=0.0;
				for ( int band=1;band<bandCount;band++ ) {
                    Audio::Wave * bw = at ( band ).data();
					v+=bw->getSample ( sample,channel );
				}
				sum.setSample ( sample, channel, v*f );
			}
		}
	}

	void getAverage(WavGroup& bandsAveraged, int samples,int downsample=1)
	{
        bandsAveraged.clear();
		if ( samples<=0 ) {
			return;
		}
        int bandCount = count();
#pragma omp parallel for
		for ( int band=0;band<bandCount;band++ ) {
            FloatWave* p = new FloatWave();
            calcAverage ( *at ( band ),*p, samples , downsample);
            bandsAveraged.append ( QSharedPointer<Wave>(p) );
		}
	}

	virtual bool valid()
	{
		if (this == 0) {
            //LogMsgA("E_ NULL WavGroup!?");
			return false;
		}
        int bandCount = count();
		for ( int band=0;band<bandCount;band++ ) {
            Wave* w = at(band).data();
			if (!w) {
                //LogMsgA("E_ NULL in WavGroup!?");
				return false;
			}
		}
		return true;
	}

    /** maximum frame count of a member */
    virtual long getFrameCount()
	{
        int bandCount = count();
		if (bandCount<1) return 0;
		long ma = 0;
		for ( int band=0;band<bandCount;band++ ) {
            Wave* w = at(band).data();
			if (w) {
                long n = w->getFrameCount();
				if (n>ma) ma=n;
//			} else {
//				LogMsgA("E_ NULL in WavGroup!?");
			}
		}
		return ma;
	}

    /** maximum channel count of a member */
	virtual int getChannels()
	{
        int bandCount = count();
		if (bandCount<1) return 0;
		long ma = 0;
		for ( int band=0;band<bandCount;band++ ) {
            Wave* w = at(band).data();
			if (w) {
				long n = w->getChannels();
				if (n>ma) ma=n;
//			} else {
//				LogMsgA("E_ NULL in WavGroup!?");
			}
		}
		return ma;
	}

	/** convenience only. not neccessarily the fs of all waves. */
	virtual double getSamplingFrequency()
	{
        int bandCount = count();
		if (bandCount<1) return 22050.0;		
        for (int band=0;band<bandCount;band++) {
            Wave* w = at(band).data();
			if (w) {		
				return w->getSamplingFrequency();
//			} else {
//				LogMsgA("E_ NULL in WavGroup!?");
			}
		}
		return 22050.0;
	}
    
    virtual int getCount()
    {
        return count();
    }
};

}
