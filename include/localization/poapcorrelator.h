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
#ifndef POAPCORRELATOR_H
#define POAPCORRELATOR_H

#include <QMap>
#include <QVector>
#include <QSharedPointer>

#include "wave/wave.h"
#include "neuro/poapifier.h"

class PoAPCorrelator : public PoAPifier
{
public:
    PoAPCorrelator(int t);

    void setCorrelationFraming(double len_ms, double step_ms,int t=1);

    virtual void calc(const float* data, unsigned int frames,  bool continued);

    void setBandSum(bool e=true) {
        bandsum=e;
    }

    void initCorrelation();


protected:
    int maxCorrelationValues;
    QMap< uint32_t, QSharedPointer<Audio::Wave> > correlates;

    double correlationSizeMs,correlationStep;
    int windowType;
    int correlationCount,windowStep;
    int nextCenterOffset;
    QSharedPointer< Neuro::SpikeWavCorrelator > correlator;
    int firstCenter;
    int maxTdoa;
    bool bandsum;
};

#endif // POAPCORRELATOR_H
