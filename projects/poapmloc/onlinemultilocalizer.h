#ifndef OLINEMULTILOCALIZER_H
#define OLINEMULTILOCALIZER_H
#pragma once

#include "wavio/listenerthreadf.h"
#include "localization/poaplocalizergeneric.h"
#include "angulardetection.h"
#include "angulargaussian.h"
#include <QObject>
#include <QList>
#include <QVector>
#include <QString>
#include <QElapsedTimer>

class OlineMultiLocalizer : public QObject
{
    Q_OBJECT

protected:
    QList < PoAPLocalizerGeneric* > cor;
    int frames,channels;
    bool done;
    QList< QVector<int> > arrayChannels;
    float* buf;
    ListenerThreadF* input;
    bool argmax,sumbands,em;
    double minLikelihood;
    double frameTime;
    QVector < QList < AngularGaussian > > clusters;
    int arrayOffset;
    double timeOffset;
    QElapsedTimer timer;
    int port;
    int debug,framesDone,framesMax;
    int frameSkip;
    QList<double> emparam;
	int linesout;
	double lastEmTime;
	double emTimeWindow;
	QVector< QList <  LocalizedSpectrum > > history;
    bool quiet;

public:
    explicit OlineMultiLocalizer(QObject *parent = 0);

    virtual ~OlineMultiLocalizer() {
        foreach (PoAPLocalizerGeneric* p,cor) {
            delete p;
        }
        cor.clear();
    }

    void add(PoAPLocalizerGeneric* p,QVector<int> channels) {
        cor.append(p);
        arrayChannels.append(channels);
    }

    QVector<double> getCenterFrequencies() {
        return cor.first()->getCenterFrequencies();
    }

	void init(ListenerThreadF* l, int skip, int o=0, double t=0);

    void start() ;

    bool isDone() {
        return done;
    }

    double getTime() {
        return cor.at(0)->getInputTime();
    }

    void setQuiet(bool b=true) {
        quiet=b;
    }

    void setMinLikelihood(double l) {
        minLikelihood=l;
    }

    void  setDebug(int l)  {
        debug=l;
    }

    void setFrameLimit(int l) {
        framesMax=l;
    }

    void setArgmax(int l,bool bandsum=true)  {
        argmax   = l;
		sumbands = bandsum;
    }

    void setEM(double timeWindow,QList<double> & l)  {
        em = true;
		emTimeWindow=timeWindow;
        emparam = l;
    }


protected:
    double emCluster(QList<LocalizedSpectrum> res, int arrayIndex, QList<AngularGaussian> & clusters);

    inline bool checkEmParam(const AngularGaussian & a);

public slots:
    void calc(float* data,bool continued=true);    
    void stopped();
    void error(QString);



};

#endif // OLINEMULTILOCALIZER_H
