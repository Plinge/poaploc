#pragma once
#include "APTL/aplogfmt.h"

#include "neuro/jeffresscorrelator.h"

namespace Neuro {


class RTN1
{
public:
    double value;
    double a,b;

    RTN1() : value(0.0), a(1.0), b(0.1)
    {
    
    }
	void setup(double a1,double b1)
	{
		a=a1;b=b1;
	}

    double calc(double x)
    {
        double y = (a*x) + (b*x*value);
        value=y;
        return y;
    }


};

class RTN
{
public:
    double value;
    double a,b;
	DelayFilter delay;

    RTN(int d) : value(0.0), a(1.0), b(0.1)
    {
		delay.SetLength(d);
    }

    virtual ~RTN()
    {
    
    }
    
    double calc(double x)
    {
        double y = a * x + b * x * delay.Calc(value);
        value = y;
        return y;
    }

	void setAlpha(double d) 
	{
		a=d;
	}

	void setBeta(double d) 
	{
		b=d;
	}

	void reset()
	{
		value=0.0;
	}
};

class RTNNPitchRow
{
public:
    
    AP_ArrayPtr<RTN> rtnn;
    int tmin,tmax,tstep,n;

    RTNNPitchRow(int t0=50,int t1=200,int s=4) : tmin(t0),tmax(t1),tstep(s)
    {
		if (s<1) s=1;
        for (int i=t1;i>=t0;i-=s) {
            rtnn.AddTail( new RTN(i));
        }
		n = rtnn.GetCount();
    }

    virtual ~RTNNPitchRow()
    {
        rtnn.DeleteAll();
    }

	void setAlphas(double a0,double a1) 
	{
		for (int i=0;i<n;i++) {
			rtnn[i].setAlpha( a0 + ((a1-a0)*i)/n );
		}
	}
	void setAlpha(double a) 
	{
		for (int i=0;i<n;i++) {
			rtnn[i].setAlpha( a );
		}
	}
	void setBetas(double a0,double a1) 
	{
		for (int i=0;i<n;i++) {
			rtnn[i].setBeta( a0 + ((a1-a0)*i)/n );
		}
	}
	void setBeta(double b) 
	{
		for (int i=0;i<n;i++) {
			rtnn[i].setBeta( b );
		}
	}

	int getChannels()
	{
		return n;
	}

    void calc(double x)
    {
        for (int i=0;i<n;i++) {
			rtnn[i].calc(x);
		}
    }

	void reset()
	{
		for (int i=0;i<n;i++) {
			rtnn[i].reset();
		}
	}
    double getValue(int channel)
    {
        return rtnn[channel].value;
    }
};

 

class RTNNCorRow: public CorrelatorRow
{
public:

    AP_ArrayPtr<RTN1> rtnn;
     

	RTNNCorRow(int n) : CorrelatorRow(n)
    {
        for (int channel=0;channel<n;channel++) {
			rtnn.AddTail( new RTN1() );
		}
    }

	void setup(double a,double b)
	{
		for (int channel=0;channel<n;channel++) {
            rtnn[channel].setup(a,b);
		}
	}

    virtual ~RTNNCorRow()
    {
        rtnn.DeleteAll();
    }

    inline void calc(double l,double r)
    {
        put2(l,r);rewind();
        for (int channel=0;channel<n;channel++) {
            rtnn[channel].calc( nextProd() );
        }
    }

    inline double getValue(int channel)
    {
        return rtnn[channel].value;
    }
};

class PitchLabeler : public Audio::Labeler
{
protected:
	int t0,t1,s;
	double fs;

public:
	PitchLabeler() : fs(22050.0), t0(50), t1(300), s(1)
	{
		
	}
	void setup(int a,int b,int c)
	{
		t0=a;t1=b;s=c;
	}
	
	void setFs(double f)
	{
		fs=f;
	}

	virtual AP_String getLabel(double index) {				
		return getLabelForValue(getValue(index));	
	}
	virtual AP_String getLabelForValue(double value) {
		AP_String str; 
		str.Format("%.1f", fs / (t1-value*s) ); 
		return str;
	}
     virtual AP_String getLabelTitle(bool shrt = true)
	 {
		 if (shrt) {
			return "p [Hz]";
		 }
		 if (APLanguage::is("de")) {
			 return "Pitch [Hz]";
		}
		return "pitch [Hz]";
	 }
	virtual double getValue(double index) { return index; }
	virtual bool isContinous() { return false; }
};


class RTNNPitchWavCorrelator
{
	RTNNPitchRow* row;
	PitchLabeler labler;
	double alpha,beta0,beta1;

public:
	    
	
    RTNNPitchWavCorrelator() : row(0), alpha(1.0), beta0(0.0), beta1(0.0)
    {

    }

    virtual ~RTNNPitchWavCorrelator()
    {
        if (row!=0) {
            delete row;
            row=0;
        }
        
    }
	PitchLabeler* getLabeler()
	{
		return &labler;
	}

	int getChannels()
	{
		if (!row) return 0;
		return row->getChannels();
	}
	void setAB(double a,double b1,double b2)
	{
		alpha=a;
		beta0=b1;
		beta1=b2;
	}

	void setup(int t0=50,int t1=200,int s=1)
	{
		if (row!=0) {
			delete row;
		}
		row = new RTNNPitchRow(t0, t1, s);
		row->setBetas(beta0, beta1);
		row->setAlpha(alpha);
		labler.setup(t0,t1,s);
	}	

	bool calc(Audio::AP_wav&in,int channel,Audio::AP_wav&out)
    {
		if (row==0) {
			return false;
		}
		int channels = row->getChannels();
		int sampleCount = in.GetSampleCount();
		LogFmtA("PitchCorrelator.calc %D channels for %D samples",channels, in.GetSampleCount());
        out.Create(in.GetFreq(),sampleCount* channels*2,16, channels);
		labler.setFs(in.getSamplingFrequency());
		static const double f1=1.0/32768.0;
		static const double f2=16384.0;
		row->reset();
        for (int sampleIndex=0;sampleIndex<sampleCount;sampleIndex++) {
            row->calc( in.GetSampleValue(sampleIndex,channel)*f1);
			for (int channel=0;channel<channels;channel++) {
                out.SetSampleValue(sampleIndex,clip15s(row->getValue(channel)*f2),channel);
            }             
        }
		in.CopyCuesTo(&out);
        return true;
    }

  
     
};

}