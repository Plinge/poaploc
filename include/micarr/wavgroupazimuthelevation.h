#pragma once
#include "wave/wavgroup.h"
#include "localization/azimuthelevationbackprojection.h"

class WavGroupAzimuthElevation : public Audio::WavGroup
{
	AzimuthElevationBackprojection* projection;
public:

    WavGroupAzimuthElevation(AzimuthElevationBackprojection* p) : projection(p), Audio::WavGroup()
    {

    }

    virtual ~WavGroupAzimuthElevation()
    {

    }

};
