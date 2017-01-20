#ifndef HISTOGRAM_H
#define HISTOGRAM_H

/**
  * a runnning historgram over the last n samples
  */
class Histogram
{
protected:
    double ma,ma2;
    double mi,mi2;
    double resolution;
    int bucketCount;
    int sampleCount;
    int sampleLimit;
    int sampleCountTotal;
    int *bucket;
    int *indexHistory;
    int samplesForgotten;
    int indexHistoryPos;  //< ringbuffer position

public:

    Histogram(double lo, double hi,int bucks, int sams);

    virtual ~Histogram();

    int getSamplesForgotten() const
    {
        return samplesForgotten;
    }

    int getSampleCountOver() const
    {
        return bucket[bucketCount];
    }

    int getSampleCountUnder() const
    {
        return bucket[-1];
    }

    int getSampleCountTotal() const
    {
        return sampleCountTotal;
    }

    int getSampleCount() const
    {
        return sampleCount;
    }

    int value2index(double value) const;

    double index2value(int index) const;

    virtual void reset();

    /**
      * init with some values between vlo and vhi
      */
    void init(double  vlo,double vhi);

    /**
      * average
      */
    double getMean();

    /**
      * update by value
      */
    virtual int update(double value);

    virtual double probLeIndex(double prob) const;

    inline double probLeValue(double prob) const
    {
        return index2value(probLeIndex(prob));
    }

    virtual double probGeIndex(double prob) const;

    inline double probGeValue(double prob) const
    {
        return index2value(probGeIndex(prob));
    }

    virtual int getCount(int bin)
    {
        return bucket[bin];
    }
};


class SparseHistogram: public Histogram
{
protected:
    int *indexHistoryAge;
    int  ageLimit;

public:
    SparseHistogram(double lo=0, double hi=100,int bucks=10, int sams=0,int age=0);
    virtual ~SparseHistogram();
    virtual void step(int inc=1);
    virtual void reset();
    virtual int update(double value);

};

#endif // HISTOGRAM_H
