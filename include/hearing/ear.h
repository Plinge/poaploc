#pragma once
#include <math.h>

namespace Hearing
{
class Ear
{
public:
    /** trf in after Terhardt (1979) */
    static double outerMiddleTransferTerhardt(double f)
    {
        f = f*1e-3; // kHz
        double dB =  -3.64 * pow(f, -0.8) + 6.5* exp( -0.6 *  pow(f - 3.3,2.0) ) - 1e-3 * pow(f,4);
        return pow(10.0,dB/20.0);
    }

};

}
