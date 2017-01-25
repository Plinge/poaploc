#pragma once
#ifndef LOCALIZEDSPECTRA_H_
#define LOCALIZEDSPECTRA_H_
#include <QVector>
#include <QString>
#include "data/localizedspectrum.h"
#include "data/dbscan.h"

class  LocalizedSpectra
{
protected:
    QVector < QList < LocalizedSpectrum > > data;    
    double minTime;
    double maxTime;
    int maxBand;
public:
    LocalizedSpectra();
    int read(const QString & datafilename,double threshold=0.0);
    double getMaxTime();
    double getMinTime();
    int getArrayCount();
    int getMaxBand();
    QList< LocalizedSpectrum > &  getSpectra(int arrayIndex);
    void getSpectraForTimeWindow(int arrayIndex, QList< LocalizedSpectrum > &  res,double minTime,double maxTime,int minBand=2);

};

/**
 *
 * @author Axel Plinge
 */
class LocalizedSpectrumNeighbourRelation: public NeighbourRelation
{
protected:
    QVector< QList< int > > r;
    int maxIndex;

    virtual void add(int i,int j) {
        if (!r[i].contains(j)) {
            r[i].append(j);
        }

    }

public:

    virtual bool neighbour(int i,int j) {
        if (i==j) return true;
        return r.at(i).contains(j);
    }

    virtual void getNeighbours(int i,QList< int > & n)
    {
        if (i>=r.size() || i<0)
            return;
        n = r.at(i);
    }

    virtual int  getMaxIndex()
    {
        return maxIndex;
    }

    virtual void build(QList < LocalizedSpectrum > &d, double  maxAngle, double minSpectralCorrelation) {
        r.clear();
        r.reserve(d.size());        
        for (int i=0;i<d.size();i++) {
            r.append(QList<int>());
        }
        maxIndex = d.size()-1;
        for (int i=0;i<=maxIndex;i++) {
            for (int j=i+1;j<=maxIndex;j++) {
                double da = d[i].azimuthDistance(d.at(j));
                double sc = d[i].normalizedSpectralCorrelation(d.at(j));
                if (da<=maxAngle && sc>=minSpectralCorrelation) {
                    add(i,j);
                    add(j,i);
                }
            }
        }
    }
};

#endif
