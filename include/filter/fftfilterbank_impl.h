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
#pragma once
#include "fftfilterbank.h"

template <class VALUETYPE>
FftFilterbank<VALUETYPE>::FftFilterbank(int n, double f1, double fn, double fs, Filtering::Spacing s, Filtering::FilterShape m, double scale)
    : count(n), minFrequency(f1), maxFrequency(fn), samplingFrequency(fs), spacing(s), filterShape(m), scaling(scale), nu(10) //emphasis(0.0)
{
    initFilters();
}

template <class VALUETYPE>
FftFilterbank<VALUETYPE>::~FftFilterbank()
{
    clearFilters();
}

template <class VALUETYPE>
void FftFilterbank<VALUETYPE>::setNu(int nn)
{
    if (nn==nu) return;
    nu = nn;
    init();
}

template <class VALUETYPE>
void FftFilterbank<VALUETYPE>::setup(int n, double f1, double fn, double fs, Filtering::Spacing s, Filtering::FilterShape m,double scale)
{
	count=n;
	minFrequency=f1;
	maxFrequency=fn;
    scaling=scale;
	if (fs>0.0) { // VB: ref oMissing; *shudder*
		samplingFrequency=fs;
	}
    if (s!=Filtering::UNDEFINED) {
        spacing=s;
    }
    if (m!=Filtering::NOSPLIT) {
		filterShape=m;	
	}
	init();
}

template <class VALUETYPE>
void FftFilterbank<VALUETYPE>::clear()
{
    clearFilters();
}

template <class VALUETYPE>
void FftFilterbank<VALUETYPE>::clearFilters()
{
    coeffs.clear();
}

template <class VALUETYPE>
int FftFilterbank<VALUETYPE>::getCount()
{
	return count;
}

template <class VALUETYPE>
void FftFilterbank<VALUETYPE>::setSamplingFrequency(double fs)
{
	if (samplingFrequency==fs) return;
	samplingFrequency=fs;
	init();
}

template <class VALUETYPE>
double FftFilterbank<VALUETYPE>::getSamplingFrequency()
{
    return samplingFrequency; 
}

template <class VALUETYPE>
int FftFilterbank<VALUETYPE>::getOverlap()
{
	return -length2;
}

template <class VALUETYPE>
void FftFilterbank<VALUETYPE>::reset()
{
	// nothing to do
}

template <class VALUETYPE>
double FftFilterbank<VALUETYPE>::getBandGain(int index)
{
    double ma=coeffs[index][0];
    for (int i=0;i<coeffs[index].size();i++) {
        double c = fabs(coeffs[index].at(i));
        if (c>ma){
            ma=c;
        }
    }
    return ma;    
}

template <class VALUETYPE>
void FftFilterbank<VALUETYPE>::setBandGain(int index,double gain)
{
	double ma=getBandGain(index);	
	if (ma<=0.0) ma=1.0;
    addBandGain(index, gain/ma);	
}

template <class VALUETYPE>
void FftFilterbank<VALUETYPE>::addBandGain(int index,double gain)
{    
    for (int i=0;i<coeffs[index].size();i++) {
        coeffs[index][i] = (coeffs[index][i] * gain);
    }
}


template <class VALUETYPE>
void FftFilterbank<VALUETYPE>::init()
{
    initFilters();
}

template <class VALUETYPE>
void FftFilterbank<VALUETYPE>::initFilters()
{	
    clearFilters();
    FilterbankCentered<VALUETYPE>::centerFreqeuencies.clear();
    length  = 1 << nu;
	length1 = length-1;
	length2 = 1 << (nu-1);    
    window.resize(length);
    sine_window(length,window.data());
	if (samplingFrequency<=0.0) {
		return;
	}
    // treat SII as special case
    if (spacing == Filtering::SII) {
        count=Filtering::SII_CB_COUNT;
        //LogFmtA("FftFilterbank %D bands for SII CB",count);
		for (int i=0;i<count;i++) {	
            int f1 = (Filtering::SII_CB_LIMITS[i]*length)/(2.0*samplingFrequency);
            int f2 = (Filtering::SII_CB_LIMITS[i+1]*length)/(2.0*samplingFrequency);
			int coeffIndex=0;
            for (;coeffIndex<f1;coeffIndex++) {
                setCoeff(i,coeffIndex,0.0);
            }
            for (;coeffIndex<f2;coeffIndex++) {
                setCoeff(i,coeffIndex,1.0);
            }
            for (;coeffIndex<length2;coeffIndex++) {
                setCoeff(i,coeffIndex,0.0);
            }
            FilterbankCentered<VALUETYPE>::centerFreqeuencies.append((Filtering::SII_CB_LIMITS[i]+Filtering::SII_CB_LIMITS[i+1])*0.5);
		}
		return;
	}

	// empty
    if (count<2) {
        coeffs.append(QVector<float>(length));
        for (int coeffIndex=0;coeffIndex<length2;coeffIndex++) {
            setCoeff(0,coeffIndex,1.0);
        }
        FilterbankCentered<VALUETYPE>::centerFreqeuencies.append(samplingFrequency/2.0);
        return;
    }

	// setup by center frequnecies and filter shape
    //LogFmtA("FftFilterbank %D bands %.1f-%.1f",count,  minFrequency, maxFrequency);
    QVector<double> fs = FilterbankCentered<VALUETYPE>::getCenterFrequencies(count, spacing, minFrequency, maxFrequency);
    for (int i=0;i<count;i++) {
        coeffs.append(QVector<float>(length));
        FilterbankCentered<VALUETYPE>::centerFreqeuencies.append(fs[i+1]);
        switch (filterShape)
        {
        case Filtering::GAMMATONE:
        {
            double cf = fs[i+1];
            // LogFmtA("GT c = %4.2f Hz w = %.2f (GM)", cf, Hearing::Gammatones::bandwidthGlasbergMoore(cf));
            for (int coeffIndex=0;coeffIndex<length2;coeffIndex++) {
                double f = (coeffIndex*samplingFrequency) / length;
                setCoeff(i,coeffIndex,Hearing::Gammatones::amplitudeGlasbergMoore(f,cf,scaling));
            }
        }
        break;
        case Filtering::TRIANGULAR:
		{
            int f1 = ((fs[i+0])*length)/(2.0*samplingFrequency);
            int f2 = ((fs[i+1])*length)/(2.0*samplingFrequency);
            int f3 = ((fs[i+2])*length)/(2.0*samplingFrequency);
			if (f1==f2) f1--;
			if (f2==f3) f3++;
            int coeffIndex=0;
            for (;coeffIndex<f1;coeffIndex++) {
                setCoeff(i,coeffIndex,0.0);
            }
			double c1 = 1.0 / (f2-f1);
            for (;coeffIndex<f2;coeffIndex++) {
                setCoeff(i,coeffIndex, (double)(coeffIndex - f1) * c1);
            }
			double c2 = 1.0 / (f3-f2);
			for (;coeffIndex<f3;coeffIndex++) {
                setCoeff(i,coeffIndex,(double)(f3 - coeffIndex) * c2);
            }
            for (;coeffIndex<length2;coeffIndex++) {
                setCoeff(i,coeffIndex,0.0);
            }
		} break;
        default: // RECTANGULAR
        {
            int f1 = ((fs[i+0]+fs[i+1])*length)/(2.0*samplingFrequency);
            int f2 = ((fs[i+1]+fs[i+2])*length)/(2.0*samplingFrequency);
            int coeffIndex=0;
            for (;coeffIndex<f1;coeffIndex++) {
                setCoeff(i,coeffIndex,0.0);
            }
            for (;coeffIndex<f2;coeffIndex++) {
                setCoeff(i,coeffIndex,1.0);
            }
            for (;coeffIndex<length2;coeffIndex++) {
                setCoeff(i,coeffIndex,0.0);
            }
        }
        }
    }
}


