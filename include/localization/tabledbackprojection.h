#pragma once
#undef max
#undef min
#define _USE_MATH_DEFINES
#include <math.h>
#include "APTL/aplogfmt.h"
#include "audio/AP_SigProc.h"
#include "audio/AP_wav.h"
#include <algorithm>


class TabledBackprojection
{
public:
	static bool calc(Audio::AP_wav & in, Audio::AP_wav & out,ProgressIndicator* progress = 0,int bands=1,int angleStep=1)
	{			
		return true;
	}
};