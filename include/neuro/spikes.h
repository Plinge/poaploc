#pragma once
#include "audio/AP_wav.h"
#include "audio/Filter/MovingAverage.h"
#include "audio/AP_SigProc.h"
#include "neuro/spikegenerator.h"
#include "neuro/spikegeneratorhws.h"
#include "neuro/spikegeneratorzc.h"
#include "neuro/spikegeneratorpoa.h"

namespace Neuro
{

typedef enum SpikeModeEnum { None, HWS, ZCS, POA, POA2, POA3, POA4, POA5, POA6, POA7, POA8 } SpikeMode;

static const char* spikeModeName(SpikeMode m) {
    if (m== HWS) return "HWS";
    if (m== ZCS) return "ZCS";
    if (m== POA) return "POA1";
    if (m== POA2) return "PoA";
    if (m== POA3) return "PoAa";
    if (m== POA4) return "PoAl";
    if (m== POA5) return "PoAP1";
    if (m== POA6) return "PoAP2";
    if (m== POA7) return "PoAPpd";
    if (m== POA8) return "PoAPp";
    return "-";
}

static SpikeMode spikeModeByName(const char* n)
{
    AP_String name(n);
    
    if (name.CompareNoCase("HWS")==0) {
        return Neuro::HWS;
    }  
    if (name.CompareNoCase("ZCS")==0) {
        return Neuro::ZCS;
    }   
    if (name.CompareNoCase("POA")==0) {
        return Neuro::POA5;
    }        
    if (name.CompareNoCase("POA2")==0) {
        return Neuro::POA2;
    }    
    if (name.CompareNoCase("ZC-POA")==0) {
        return Neuro::POA;
    }
    if (name.CompareNoCase("POAPD")==0) {
        return Neuro::POA7;
    }
    if (name.CompareNoCase("POAP")==0) {
        return Neuro::POA8;
    }
    return Neuro::POA5;
}



class SpikeGeneratorSwitched 
{
    SpikeGeneratorPOA poa;
    SpikeGeneratorZC zc;
    SpikeGeneratorHWS hws;
    SpikeMode mode;
    double exponent;
    double spikeWidth;
    double gain;
public:
    SpikeGeneratorSwitched () : exponent(0.5), spikeWidth(50e-3), gain(1.0)
    {
    }

    virtual void setMode(SpikeMode m)
    {
        mode=m;
    }

    virtual void setExponent(double expo)
    {
        exponent=expo;
    }

    virtual void setSpikeWidth(double d)
    {    
        spikeWidth=d;
    }
    
    virtual void setGain(double d)
    {
       gain=d;
    }

    virtual void setRelativeThreshold(double db)           
    {
        poa.setRelativeThreshold(db);
    }

    bool spikify(Audio::AP_wav&in,Audio::AP_wav&out,ProgressIndicator* progress = 0) {
        SpikeGenerator* generator=0;
        switch (mode) {
        case HWS:
            generator = &hws;
            hws.setExponent(exponent);
            hws.setGain(gain);
            break;

        case POA:
            generator = &poa;
            poa.setExponent(exponent);
            poa.setSpikeWidth(spikeWidth);
            poa.setMode(0);
            poa.setGain(gain);
            break;

        case POA2:
            generator = &poa;
            poa.setExponent(exponent);
            poa.setSpikeWidth(spikeWidth);
            poa.setMode(2);
            poa.setGain(gain);
            break;
            
        case POA3:
            generator = &poa;
            poa.setExponent(exponent);
            poa.setSpikeWidth(spikeWidth);
            poa.setMode(3);
            poa.setGain(gain);
            break;
            
        case POA4:
            generator = &poa;
            poa.setExponent(exponent);
            poa.setSpikeWidth(spikeWidth);
            poa.setMode(4);
            poa.setGain(gain);
            break;
            
        case POA5:
            generator = &poa;
            poa.setExponent(exponent);
            poa.setSpikeWidth(spikeWidth);
            poa.setMode(5);
            poa.setGain(gain);
            break;

        case POA6:
            generator = &poa;
            poa.setExponent(exponent);
            poa.setSpikeWidth(spikeWidth);
            poa.setMode(6);
            poa.setGain(gain);
            break;        

        case POA7:
            generator = &poa;
            poa.setExponent(exponent);
            poa.setSpikeWidth(spikeWidth);
            poa.setMode(5,true);
            poa.setGain(gain);
            break;
            
        case POA8:
            generator = &poa;
            poa.setExponent(exponent);
            poa.setSpikeWidth(spikeWidth);
            poa.setMode(8,true);
            poa.setGain(gain);
            break;

        case ZCS:
            generator = &zc;
            zc.setExponent(exponent);
            zc.setSpikeWidth(spikeWidth);
            zc.setGain(gain);
            break;

        default:
            out = in;
            return true;
        }
        return generator->spikify(in,out,progress);
    }
};

}
