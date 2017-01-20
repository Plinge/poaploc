#pragma once
#undef max
#undef min
#define _USE_MATH_DEFINES
#include <math.h>
#include "wave/wave.h"
#include "wave/floatwave.h"
#include <algorithm>
#include <iostream>
#include <QVector>
#include <QMap>
#include <QString>

class TablularBackprojection
{
protected:
    class TimeDelayTable : public QVector<float>
	{
        int stride;
	public:
        TimeDelayTable(int dim2=0,int dim1=360) : stride(dim1), QVector<float>(dim1*dim2)
        {            
		}

		virtual ~TimeDelayTable()
		{			

		}

        void setValue(int x2, int x1,float v)
        {
            (*this)[x2*stride+x1]=v;
        }

        inline float* at(int step)
		{
            return data()+stride*step;
		}

        inline float getValue(int x2, int x1)  const
        {
            return (*this)[x2*stride+x1];
        }


	};
public:
    virtual void precalc()=0;

};

class TablularTwoDeeBackprojection : public TablularBackprojection
{
protected:

    QMap<int,TimeDelayTable> tables;

    int dimSub,dimSup;
    bool interpolate;
    bool init_done;
    int verbose;
	virtual void init()=0;

public:
    TablularTwoDeeBackprojection() : dimSub(0), dimSup(0), init_done(false), interpolate(true), verbose(0)
	{
	}

	virtual ~TablularTwoDeeBackprojection()
	{
        tables.clear();
	}

    void setVerbose(int v=1) {
        verbose=v;
    }

    void setInterpolation(bool on)
    {
        interpolate = on;
    }

	virtual void precalc()
	{
        if (tables.isEmpty()) {
			init();
		}
	}

    virtual bool calc(int idx,Audio::Wave & in, Audio::Wave & out);

    virtual bool calcFloat(int idx,Audio::FloatWave & in, Audio::FloatWave & out);


    virtual void dump(int idx,int limit=99999)
    {

        if (!tables.contains(idx)) {
            QString err = QString("Backprojection not initialized or no index %1!").arg(idx);
            throw std::runtime_error((const char*)err.toLocal8Bit());
        }

        TimeDelayTable& table = tables[idx];

        for (int d=0;d<dimSup;d++) {
            float *tdoas = table.at(d);
            for (int a=0;a<dimSub;a++) {
                float tdoa = *tdoas++;
                std::cout << d << "\t" << a << "\t" << tdoa << std::endl;
                if (limit--<0) return;
            }
        }

    }

};

 

