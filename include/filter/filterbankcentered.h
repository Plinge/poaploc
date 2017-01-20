#ifndef FILTERBANKCENTERED_H
#define FILTERBANKCENTERED_H
#pragma once
#include "filterbank.h"

/**
 * Filterbank wich center freqs and corresponding labels
 *
 * @author Axel Plinge
 *
 * @par history
 *   - 2009-12-09  axel  extracted
 *   - 2012-11-05  axel  template
 */
template <class VALUETYPE>
class FilterbankCentered : public Filterbank<VALUETYPE>
{
protected:
    QVector< double > centerFreqeuencies;

public:
    virtual double getFrequency(int index) const
    {
        return centerFreqeuencies[index];
    }

    virtual QVector< double > getFrequencies() const
    {
        return centerFreqeuencies;
    }

    FilterbankCentered()
    {
    }

    virtual ~FilterbankCentered()
    {
    }

};


#endif // FILTERBANKCENTERED_H
