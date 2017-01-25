#ifndef LOCALIZEDSPECTRUM_H
#define LOCALIZEDSPECTRUM_H
#pragma once

#include <vector>

class LocalizedSpectrum
{
public:
    double time;
    double angle;
    double elevation;
    std::vector<double> spectrum;
    int idx;

protected:
    double autocorrelation;    

public:
    LocalizedSpectrum(double t=0.0,double a=0.0,double e=0.0);
    LocalizedSpectrum(double t, double a, const std::vector<double> & s,double e=0.0,int j=0);
    LocalizedSpectrum(const LocalizedSpectrum& other);

    double getAngle() const;
    double getElevation() const;
    double getTime() const;
    int    getBandCount() const;
    double getSpectralValue(int band) const;
	void   setSpectralValue(int band,double v);
    int    getIndex() { return idx; }
    void   setIndex(int i) { idx=i; }
    void updateAutocorrelation();
    double spectralCorrelation(const LocalizedSpectrum & other) const;
    double normalizedSpectralCorrelation(const LocalizedSpectrum & other) const;
    double azimuthDistance(const LocalizedSpectrum & other) const;
    double getSpectralSum() const;
    int spectralBands(double th=0.0) const;
    
};




#endif // LOCALIZEDSPECTRUM_H
