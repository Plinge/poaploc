/*
 * Copyright 2010-2015 (c) Axel Plinge / TU Dortmund University
 *
 * ALL THE COMPUTER PROGRAMS AND SOFTWARE ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND.
 * WE MAKE NO WARRANTIES,  EXPRESS OR IMPLIED, THAT THEY ARE FREE OF ERROR, OR ARE CONSISTENT
 * WITH ANY PARTICULAR STANDARD OF MERCHANTABILITY, OR THAT THEY WILL MEET YOUR REQUIREMENTS
 * FOR ANY PARTICULAR APPLICATION. THEY SHOULD NOT BE RELIED ON FOR SOLVING A PROBLEM WHOSE
 * INCORRECT SOLUTION COULD RESULT IN INJURY TO A PERSON OR LOSS OF PROPERTY. IF YOU DO USE
 * THEM IN SUCH A MANNER, IT IS AT YOUR OWN RISK. THE AUTHOR AND PUBLISHER DISCLAIM ALL
 * LIABILITY FOR DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES RESULTING FROM YOUR USE OF THE
 * PROGRAMS.
 */

#include "histogram.h"
#include <cstdlib>
#include <qdebug.h>

Histogram::Histogram(double lo, double hi,int bucks, int sams)
{
    mi = lo;
    ma = hi;
    bucketCount = bucks;
    resolution = (ma-mi)/bucketCount;
    ma2 = ma + resolution * 0.5;
    mi2 = mi - resolution * 0.5;
    sampleLimit = sams;
    /// -1 and n+1 are a legal bucket index
    bucket = reinterpret_cast<int*>(calloc(bucketCount+2,sizeof(int)));
    bucket++;
    indexHistory = reinterpret_cast<int*>(calloc(sampleLimit,sizeof(int)));
    indexHistoryPos = sampleCount = sampleCountTotal = samplesForgotten = 0;
}

Histogram::~Histogram()
{
    bucket--;
    free(bucket);
    free(indexHistory);
}

int Histogram::value2index(double value) const
{
    if (value>ma2) return bucketCount;
    if (value<mi2) return -1;
    return static_cast<int>((value-mi)/resolution+0.5);
}

double Histogram::index2value(int index) const
{
    return mi + index * resolution;
}

void Histogram::reset()
{
    for (int index=-1;index<bucketCount+1;index++) {
        bucket[index]=0;
    }
    for (int index=0;index<sampleLimit;index++) {
        indexHistory[index]=-5;
    }
    indexHistoryPos = sampleCount = sampleCountTotal = samplesForgotten = 0;
}

void Histogram::init(double  vlo,double vhi)
{
    for (double v=vlo;v<vhi;v+=resolution) {
        update(v);
    }
}

double Histogram::getMean()
{
    double m=0;
    if (sampleCount<=0) {
        return mi;
    }
    for (int bin=0;bin<bucketCount;bin++) {
        m += bucket[bin] * index2value(bin);
    }
    m /= (double)sampleCount;
    return m;
}

int Histogram::update(double value)
{
    int idx = value2index(value);
    if (indexHistory) {
       int old_idx = indexHistory[indexHistoryPos];
       if (old_idx>=-1) {
           bucket[old_idx]--; //  out with the old
           sampleCountTotal--;
           if (old_idx>=0 && old_idx<bucketCount) {
               sampleCount--;
           }
           samplesForgotten++;
       }
       indexHistory[indexHistoryPos] = idx; //  in with the new
       indexHistoryPos++;
       if (indexHistoryPos>=sampleLimit) {
           indexHistoryPos=0;
       }
    }
    bucket[idx]++;
    if (idx>=0 && idx<bucketCount) {        
        sampleCount++;
    }
    sampleCountTotal++;
    return idx;
}

double Histogram::probLeIndex(double prob) const
{
    int n = sampleCount*prob;
    int sum=0;
    int index=0;
    for  (;index<bucketCount;index++) {
        sum += bucket[index];
        if  (sum>=n) {
            break;
        }
    }
    return index;
}

double Histogram::probGeIndex(double prob) const
{
    int n = sampleCount*prob;
    int sum=0;
    int index=bucketCount-1;
    for  (;index>=0;index--) {
        sum += bucket[index];
        if  (sum>=n) {
            break;
        }
    }
    return index;
}

///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////

SparseHistogram::SparseHistogram(double lo,double hi, int bucks,int sams,int age)
    : Histogram(lo,hi,bucks,sams), ageLimit(age)
{
    indexHistoryAge = reinterpret_cast<int*>(calloc(sampleLimit,sizeof(int)));
}

SparseHistogram::~SparseHistogram()
{
    free(indexHistoryAge);
}

void SparseHistogram::reset()
{
    Histogram::reset();
    // Additional initialization. Not strictly required
    // since indexHistory is reset to -5 when cleared.
    for (int index=0;index<sampleLimit;index++) {
        indexHistoryAge[index]=ageLimit+1;
    }
}

void SparseHistogram::step(int inc)
{
    for (int i=0;i<sampleLimit;i++) {
       int old_idx = indexHistory[i];
       if (old_idx >= -1) {
           indexHistoryAge[i]+=inc;
           if (indexHistoryAge[i]>ageLimit) {
               bucket[old_idx]--; //  out with the old
               sampleCountTotal--;
               if (old_idx>=0 && old_idx<bucketCount) {
                   sampleCount--;
               }
               indexHistory[i]=-5; // mark empty
               samplesForgotten++;
           }
       }
    }
}

int SparseHistogram::update(double value)
{
    int idx = value2index(value);
    if (indexHistory) {
        indexHistoryPos++;
        if (indexHistoryPos>=sampleLimit) {
            indexHistoryPos=0;
        }
        // check if we overwrite (should not happen too often)
        int old_idx = indexHistory[indexHistoryPos];
        if (old_idx>=-1) {
            qWarning("SparseHistogram removing history %d, buffer of %d too small!",indexHistoryPos, sampleLimit);
            bucket[old_idx]--; //  out with the old
            sampleCountTotal--;
            if (old_idx>=0 && old_idx<bucketCount) {
                sampleCount--;
            }
            samplesForgotten++;
        }
        indexHistory[indexHistoryPos] = idx; //  in with the new
        indexHistoryAge[indexHistoryPos] = 0; // fresh like sunday morning
    }
    bucket[idx]++;
    if (idx>=0 && idx<bucketCount) {
        sampleCount++;
    }
    sampleCountTotal++;
    return idx;
}
