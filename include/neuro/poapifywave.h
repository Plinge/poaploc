#ifndef POAPIFYWAVE_H
#define POAPIFYWAVE_H

#include "neuro/poapifier.h"


class PoAPifyWave : public PoAPifier
{
public:
    PoAPifyWave(const Audio::Wave & wave);

    bool process(int sz, Audio::Wave &out, double startTime=0, double endTime=-1);

    virtual int getChannelCount()
    {
        return wav->getChannels();
    }

    void setSumBands(bool b=true) {
        bandsum=b;
    }

    void setQuiet(bool q=true) {
        quiet=q;
    }

protected:
    const Audio::Wave * wav;
    bool bandsum;
    bool quiet;

};

#endif // POAPIFYWAVE_H
