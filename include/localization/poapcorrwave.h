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
#ifndef POAPCORRWAVE_H
#define POAPCORRWAVE_H
#include "localization/poapcorrelator.h"

class PoAPCorrWave : public PoAPCorrelator
{
public:
    PoAPCorrWave(const Audio::Wave &wave);

    bool process(int sz, double startTime=0.0, double endTime=-1);


    void setQuiet(bool b) {
        quiet=b;
    }

    virtual int getChannelCount()
    {
        return wav->getChannels();
    }

protected:
    const Audio::Wave* wav;
    bool quiet;
};

#endif // POAPCORRWAVE_H
