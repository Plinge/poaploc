#include "localizedspectrum.h"
#include "micarr/anglemath.h"
#include "math.h"


LocalizedSpectrum::LocalizedSpectrum(double t, double a, double e)
    : time(t), angle(a), elevation(e), idx(0)
{
}

LocalizedSpectrum::LocalizedSpectrum(double t, double a, const std::vector<double> & s, double e,int j)
    : time(t), angle(a), spectrum(s), elevation(e), idx(j)
{
    updateAutocorrelation();
}

LocalizedSpectrum::LocalizedSpectrum(const LocalizedSpectrum& other)
    : time(other.time), angle(other.angle), spectrum(other.spectrum), elevation(other.elevation), idx(other.idx)
{
    updateAutocorrelation();
}

double LocalizedSpectrum::spectralCorrelation(const LocalizedSpectrum & other) const
{
    double sc=0.0;
    for (int index=0;index<spectrum.size();index++) {
        sc += spectrum[index] * other.spectrum[index];
    }
    return sc;
}

double LocalizedSpectrum::normalizedSpectralCorrelation(const LocalizedSpectrum & other) const
{
    double sc = spectralCorrelation(other);
    double ac = sqrt(other.autocorrelation*autocorrelation);
    if (sc<=0) return 0.0;
    return sc/ac;
}

double LocalizedSpectrum::azimuthDistance(const LocalizedSpectrum & other) const
{
    return fabs(diff360(angle,other.angle));
}

double LocalizedSpectrum::getSpectralSum() const
{
    double sc=0.0;
    for (int index=0;index<spectrum.size();index++) {
        sc +=  spectrum[index];
    }
    return sc;
}


double LocalizedSpectrum::getElevation() const
{
    return elevation;
}

double LocalizedSpectrum::getAngle() const
{
    return angle;
}

double LocalizedSpectrum::getTime() const
{
    return time;
}

int LocalizedSpectrum::getBandCount() const
{
    return spectrum.size();
}

double LocalizedSpectrum::getSpectralValue(int band) const
{
    return spectrum.at(band);
}

void LocalizedSpectrum::setSpectralValue(int band,double v) 
{
	spectrum[band]=v;
}

int LocalizedSpectrum::spectralBands(double th) const
{
    int sc=0;
    for (int index=0;index<spectrum.size();index++) {
        if (spectrum[index]>th) {
            sc++;
        }
    }
    return sc;
}

void LocalizedSpectrum::updateAutocorrelation()
{
    autocorrelation = spectralCorrelation(*this);
}

