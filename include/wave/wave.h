#pragma once
#undef min
#undef max
#include <algorithm>
#include <stdexcept>

namespace Audio
{

class WaveException : public std::runtime_error
{
public:
    WaveException(const char* text = "WaveException") : std::runtime_error(text)
    {}
};

class WaveIndexOutofRangeException : public WaveException
{
public:
    WaveIndexOutofRangeException(const char* text="WaveIndexOutofRangeException") : WaveException(text)
    {}
};

/**
* @ingroup audio
* @brief interface to multichannel audiodata.
*
* @author Axel Plinge
*
* @par history
*  -  14.07.2010  Axel Plinge  created
*/
class Wave
{
public:
    Wave()
    {}

    virtual ~ Wave()
    {}

    virtual double getSample(long p,int chan) const = 0;
    virtual double getSampleOrZero(long p,int chan)
    {
        if (p<0) return 0.0;
        if (p>=getFrameCount()) return 0.0;
        return getSample(p,chan);
    }

    virtual void setSample(long p,int chan,double v) = 0;
    virtual void setSampleCount(long n)=0;
    virtual double getSamplingFrequency() const = 0;
    virtual int getFrameCount() const =0;
    virtual int getChannels() const =0;
    virtual void create(double f, int ch, int s,bool dirty=false)=0;
    virtual void zero() =0;

    /**
      * find the next position after p where a sample is not equal to zero.
      * @param p     frame position to start from
      * @param chan  channel to use
      * @param hint  internal marker, init with -1 and pass to subsequent calls
      */
    virtual int getNextNonzeroIndex(long p,int chan, int & hint,double & v) const =0;

    /**
      * @param p     frame position to start from
      * @param chan  channel to start from
      * @param hint  internal marker, init with -1 and pass to subsequent calls
      */
    virtual int getNextNonzeroIndexChannel(long p,int & chan, int & hint,double & v) const
    {
        int nChannels = getChannels();
        while (chan<nChannels) {
            int index = getNextNonzeroIndex(p, chan, hint, v);
            if (index>=0) return index;
            chan++;
            hint=-1;
            p=-1;
        }
        return -1;
    }

    virtual int getNonzeroSampleCount(int channel=-1) const
    {
        int count = 0;
        if (channel<0) {
            for (int ch=0;ch<getChannels();ch++)  {
                count += getNonzeroSampleCount(ch);
            }
        } else  {
            int index=-1,hint=-1;
            double v;
            while ((index=getNextNonzeroIndex(index, channel, hint, v))>=0) {
                count++;
            }
        }
        return count;
    }

    virtual double getMaxSample(int channel=-1) const = 0;

	virtual int getDataSamplesTotal() const
	{
        return getFrameCount()*getChannels();
    }

    virtual int getDataSamples(int channel) const
	{
        return getFrameCount();
	}

    virtual bool isSparse() const
	{
		return false;
	}

    virtual bool isInfinite() const
	{
		return false;
	}

    virtual int getTypeNumber() const
    {
        return 0;
    }

    inline double sampleToSec(long p) const
    {
        return (double)p/(double)getSamplingFrequency();
    }

    virtual double getTotalSeconds() const
    {
        return (double) getFrameCount() /  getSamplingFrequency();
    }
	
    virtual void copyTo(Wave * dst)
    {
       dst->create(getSamplingFrequency(), getChannels(), getFrameCount() );
		   copySamplesTo(dst);
    }

    virtual void copySamplesTo(Wave * dst,long maxpos=-1,long offset=0,long dstoffset=0) const
	{	
        long len = std::min(getFrameCount()-offset,dst->getFrameCount()-dstoffset);
		if (maxpos>0) {
			len = std::min(len,maxpos);
		}			
		int nc=getChannels();	
#pragma omp parallel for
		for (int c=0;c<nc;c++) {
			for (long p=0;p<len;p++) {			
                dst->setSample(p+dstoffset,c, getSample(p+offset,c));
			}
		}
	}

    virtual	void copyChannelTo(Wave * dst,int iChan=0,long maxpos=-1,int dstChan=0) const
	{
		if (!dst) return;
		if (iChan<0 || iChan>=getChannels()) {
            throw std::runtime_error("Channel index out of range in AP_wav.CopyChannelTo!");
		}
		if (dstChan<0 || dstChan>=dst->getChannels()) {
            throw std::runtime_error("Destination channel index out of range in AP_wav.CopyChannelTo!");
		}		
        long len = std::min(getFrameCount(),dst->getFrameCount());
		if (maxpos>0) {
			len = std::min(len,maxpos);
		}		
		for (long p=0;p<len;p++) {
			dst->setSample(p, dstChan, getSample(p,iChan));
		}	
	}

	virtual	void copyChannelReverse(Wave * dst,int iChan=0,long maxpos=-1,int dstChan=0)
	{
		if (!dst) return;
		if (iChan<0 || iChan>=getChannels()) {
            throw std::runtime_error("Channel index out of range in AP_wav.CopyChannelTo!");
		}
		if (dstChan<0 || dstChan>=dst->getChannels()) {
            throw std::runtime_error("Destination channel index out of range in AP_wav.CopyChannelTo!");
		}		
        long len = std::min(getFrameCount(),dst->getFrameCount());
		if (maxpos>0) {
			len = std::min(len,maxpos);
		}
		long q = len-1;
		for (long p=0;p<len;p++) {
			dst->setSample(q--, dstChan, getSample(p,iChan));
		}	
	}


	virtual void multiplyBy(double v,int cg=-1)
	{
        long ns=getFrameCount();
		if (cg<0) {
			int nc=getChannels();	
#pragma omp parallel for
			for (int c=0;c<nc;c++){
				for (long p=0;p<ns;p++)	{
					setSample(p,c,getSample(p,c)*v);
				}
			}	
		} else {
			for (long p=0;p<ns;p++)	{
				setSample(p,cg,getSample(p,cg)*v);
			}
		}
	}

    virtual void clip(double mi,double ma)
    {
        long ns=getFrameCount();
        int nc=getChannels();
#pragma omp parallel for
        for (int c=0;c<nc;c++){
            for (long p=0;p<ns;p++)	{
                double v = getSample(p,c);
                if (v>ma) v=ma;
                if (v<mi) v=mi;
                setSample(p,c,v);
            }
        }
    }

    virtual void normalize()
    {
        double d = getMaxSample();
        if (d>0.0) {
            multiplyBy(1.0/d);
        }
    }


    virtual double getSampleSum(int chan=-1)
    {
        double sum=0.0;
        if (chan<0) {
            for (int channel=0;channel<getChannels();channel++) {
                sum +=  getSampleSum(channel);
                return sum;
            }
        } else {
            for  (int index=0;index<getFrameCount();index++) {
                sum += getSample(index,chan);
            }
        }
        return sum;
    }

};

}
