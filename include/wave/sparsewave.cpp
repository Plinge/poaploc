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

#include "sparsewave.h"
#include <assert.h>

using namespace Audio;

void SparseWave::create(double freq,int chan,int vs,bool dirt)
{
    virtualSampleCount=vs;
    samplingFrequency=freq;
    if (data.count()<chan) {
        channels=0;
        foreach(ChannelData*p,data) {
            delete p;
        }
        data.clear();
        data.reserve(chan);
        for (int channel=0;channel<chan;channel++) {
//            ChannelData *c = new ChannelData();
//            if (prealloc>0) {
//                c->prealloc(prealloc, prealloc_s);
//            }
//            data.append(c);
            data.append( new ChannelData());
        }
        channels=chan;
        actualSampleCount=0;
        dirty=false;
        return;
    }
    if (!dirt) {
        for (int channel=0;channel<channels;channel++) {
            data.at(channel)->clear();
        }
        actualSampleCount=0;
        dirty=false;
    }
    channels=chan;
}

void SparseWave::zero()
{
    for (int channel=0;channel<channels;channel++) {
        data.at(channel)->clear();
    }
    actualSampleCount=0;
    dirty=false;
}

int SparseWave::getDataSamples(int channel) const
{
    return data.at(channel)->count();
}

int SparseWave::getDataSamplesTotal() const
{
    int m = 0;
    for (int channel=0;channel<channels;channel++) {
        m+=data.at(channel)->count();
    }
    return m;
}

double SparseWave::getSample(long p,int chan) const
{
    if (chan<0 || chan>=channels) {
        //            AP_String s;
        //            s.Format("SparseWave.getSample(%d,%d) => no such channel!",p,chan);
//            LogFmtA("E_ %s",(const char*)s.data());
        throw WaveIndexOutofRangeException();
        return 0.0;
    }
    ChannelData* c = data.at(chan);
    if (!c->contains(p)) return 0.0;
    return (*c)[p];
}

double SparseWave::getSampleSum(int chan) const
{
    double sum=0.0;
    if (chan<0) {
        for (int channel=0;channel<getChannels();channel++) {
            sum +=  getSampleSum(channel);
            return  sum;
        }
    } else {
        ChannelData* c = data.at(chan);

        for  (ChannelData::const_iterator it=c->constBegin();it != c->constEnd();++it) {
            sum += it.value();
        }
    }
    return sum;
}

void SparseWave::calcAverage1(Wave &out)
{
    double f = 1.0 / (double)virtualSampleCount;
    out.create(samplingFrequency*f, channels, 1, true);
    for (int channel=0;channel<channels;channel++) {
        double sum = 0.0;
        ChannelData* c = data.at(channel);
        for  (ChannelData::const_iterator it=c->constBegin();it != c->constEnd();++it) {
            sum += it.value();
        }
        out.setSample(0,channel,sum*f);
    }
}

void SparseWave::setSample(long p, int chan, double v)
{
    ChannelData* c = data.at(chan);
    if (c==0) {
        //            AP_String s;
        //            s.Format("SparseWave.setSample(%d,%d) => no such channel!",p,chan);
        //            LogFmtA("E_ %s",(const char*)s.data());
        throw WaveIndexOutofRangeException();
        return;
    }
    if (v==0.0) {
        int j = c->key(p,-1);
        if (j<0) return;
        c->remove(j);
        dirty=true;
        return;
    }
    c->insert(p,v);
    if (p>=virtualSampleCount) {
        //LogFmtA("W_ SparseWave.setSample(%d,%d) => growing size from %d.",p,chan,virtualSampleCount);
        virtualSampleCount=p+1;
    }
    //if (c->GetCount() > virtualSampleCount) {
    //LogFmtA("W_ SparseWave.setSample(%d,%d) => internal size %D does not match %D max size.",p,chan,c->GetCount(),virtualSampleCount);
    //}
    dirty=true;
}

int SparseWave::getNextNonzeroIndex(long p, int chan, int &hint, double &v) const
{
    ChannelData* c = data.at(chan);
    QList<long> keys = c->keys();
    int count = keys.size();
    if (hint<0) {
        hint = count;
        for (int i=0;i<count;i++) {
            if (keys.at(i) > p) {
                hint=i;
                break;
            }
        }
    } else  {
        hint++;
    }
    while (hint<count) {
        long pk = keys.at(hint);
        if (pk>p) {
            v = c->value(pk,0.0);
            return pk;
        }
        hint++;
    }
    return -1;
}

void SparseWave::copySamplesTo(Wave *dst, long maxpos)
{
    long len = std::min<long>(getFrameCount(),dst->getFrameCount());
    if (maxpos>0) {
        len = std::min<long>(len,maxpos);
    }
    dst->zero();
    for (int channel=0;channel<channels;channel++) {
        ChannelData* c = data.at(channel);
        for  (ChannelData::const_iterator it=c->constBegin();it != c->constEnd();++it) {
            dst->setSample( it.key() ,channel, it.value() );
        }
    }
}

double SparseWave::getMaxSample(int channel) const
{
    double m = 0.0;
    if (channel<0)  {
        for (int ch=0;ch<getChannels();ch++) {
            double x = getMaxSample(ch);
            if (x>m )m=x;
        }
        return m;
    }
    ChannelData* c = data.at(channel);
    for  (ChannelData::const_iterator it=c->constBegin();it != c->constEnd();++it) {
        double x = fabs(it.value());
        if (x>m )m=x;
    }
    return m;
}

void SparseWave::multiplyBy(double v, int ch)
{
    if (ch>=0) {
        ChannelData* c = data.at(ch);
        for  (ChannelData::iterator it=c->begin();it != c->end();++it) {
            it.value() *= v;
        }
        return;
    }
    for (int channel=0;channel<channels;channel++) {
        multiplyBy(v,channel);
    }
}

void SparseWave::shiftBy(int samples,bool dirt)
{
    for (int channel=0;channel<channels;channel++) {
        ChannelData* c = data.at(channel);
        if (!c) continue;
        int ks = c->size();
        for (int k=0;k<ks;) {
            int oldTimeKey = c->keys().at(k);
            double v= c->value(oldTimeKey);
            int newTimeKey = oldTimeKey - samples;
            c->remove(oldTimeKey);
            if (oldTimeKey<0) {
                ks--; // count decreased by one, check same index
            } else {
                c->insert(newTimeKey,v);
                k++; // check next index
            }
            assert(ks == c->size());
        }
    }
}
