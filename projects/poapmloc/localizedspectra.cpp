#include "localizedspectra.h"
#include "micarr/anglemath.h"
#include "math.h"
#include <QFile>
#include <QDebug>
#include <QStringList>

LocalizedSpectra::LocalizedSpectra() :
    minTime(0.0), maxTime(0.0), maxBand(16)
{

}

int LocalizedSpectra::read(const QString & datafilename,double threshold)
{
    QFile file(datafilename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Could not open file '" << datafilename << "'!";
        return 12;
    }
    QTextStream in(&file);
    QString line = in.readLine();
    QStringList headers = line.split("\t");
    int timeIndex = headers.indexOf("time");
    int firstValueIndex = headers.indexOf("s0");
    int lastValueIndex = firstValueIndex+1;
    int columns = headers.size();
    while (lastValueIndex<columns-1) {
        if (!headers[lastValueIndex+1].startsWith('s'))
            break;
        int nn = headers[lastValueIndex+1].mid(1).toInt();
        if (nn<=0)
            break;
        lastValueIndex++;
    }
    maxBand = lastValueIndex-firstValueIndex;
    int angleIndex = headers.indexOf("angle");
    int arrayIndex = headers.indexOf("micarray");
    if (arrayIndex<0) arrayIndex = headers.indexOf("array");
    if (timeIndex<0 ||  angleIndex<0 || firstValueIndex<0) {
        qDebug() << "Could not import file, missing columns!";
        return 3;
    }
    minTime=1e6;
    maxTime=0;
    int lineNumber=0;
    while (!in.atEnd()) {
        QString line = in.readLine();
        ++lineNumber;
        QStringList values = line.split("\t");       
        if (values.size()<=lastValueIndex) {
            qDebug() <<  datafilename << " entry "  << lineNumber << " has not enough colums! " << line;
        } else {
            int arrayNumber = arrayIndex>=0 ? values[arrayIndex].toInt() : 0;
            while (arrayNumber >=  data.size()) {
                data.append(QList<LocalizedSpectrum>());
            }
            double time = values[timeIndex].toDouble();
            if (time<minTime) {
                minTime=time;
            }
            if (time>maxTime) {
                maxTime=time;
            }
            double angle = values[angleIndex].toDouble();
            LocalizedSpectrum l(time,angle);
            for (int index=firstValueIndex;index<=lastValueIndex;index++) {
                l.spectrum.push_back( values [index].toDouble() );
            }
            l.updateAutocorrelation();
            if (l.getSpectralSum()>=threshold) {
                data[arrayNumber].append(l);
            }
        }
    }
    file.close();
    return 0;
}

double LocalizedSpectra::getMaxTime()
{
    return maxTime;
}

double LocalizedSpectra::getMinTime()
{
    return minTime;
}


int LocalizedSpectra::getArrayCount()
{
    return data.size();
}

int LocalizedSpectra::getMaxBand()
{
    return maxBand;
}

QList< LocalizedSpectrum > &  LocalizedSpectra::getSpectra(int arrayIndex)
{
    return data[arrayIndex];
}

void LocalizedSpectra::getSpectraForTimeWindow(int arrayIndex, QList< LocalizedSpectrum > &  res,double minT,double maxT,int minBand)
{
    for (int index=0;index<data[arrayIndex].size();index++) {
        double time = data[arrayIndex][index].time;
        if (time>=minT && time<maxT &&  data[arrayIndex][index].spectralBands()>=minBand) {
            res.append(data[arrayIndex].at(index));
        }
    }
}

