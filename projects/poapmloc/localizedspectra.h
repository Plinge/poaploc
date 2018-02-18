#pragma once
#ifndef LOCALIZEDSPECTRA_H_
#define LOCALIZEDSPECTRA_H_
#include <QVector>
#include <QString>
#include "localizedspectrum.h"

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

#endif
