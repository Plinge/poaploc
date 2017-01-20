#pragma once
#include "wave/wave.h"

namespace Neuro
{
class SpikeGenerator
{
public:
    virtual bool spikify(Audio::Wave&in, Audio::Wave&out)=0;
 
};

}
