#pragma once
#include "wave/wavgroup.h"
#include "wave/sparsewave.h"
#include "localization/azimuthelevationbackprojection.h"
#include "micarr/wavgroupazimuthelevation.h"
#include "wave/average.h"

class AzimuthElevationFilter
{
public:
    typedef enum { NO_POST=0, 
		POST_AZPOAP, POST_AZPOA, POST_AEGRID,
		POST_AVG, POST_SUME, POST_MAXE, POST_SUMB, 
		POST_SUME_AZPOAP, POST_MAXE_AZPOAP, POST_MAXE_AZPOA, POST_ARGMAX, POST_SUME_ARGMAX, 
		POST_THRESHOLD, POST_THRES_BANDC
	} PostFilterMode;
protected:
    int azimuthCount;
    double azimuthStep;
    int elevationCount;
    int sampleCount;
    AzimuthElevationBackprojection* backPro;
    WavGroupAzimuthElevation* bands;
    WavGroupAzimuthElevation* post;

public:
    AzimuthElevationFilter(WavGroupAzimuthElevation* b, AzimuthElevationBackprojection* p) : bands(b), backPro(p), post(0)
    {

    }

    virtual ~AzimuthElevationFilter()
    {
        clear();
    }

    inline double getSample(Audio::Wave& w,int sample,int azimuth,int elevation)
    {
        //return w.getSample(sample, elevation*azimuthCount+azimuth);
        return w.getSample(sample, backPro->getChannelIndex(elevation, azimuth));
    }

    inline void  setSample(Audio::Wave& w,int sample,int azimuth,int elevation,double v)
    {
        //w.setSample(sample, elevation*azimuthCount+azimuth,v);
        w.setSample(sample, backPro->getChannelIndex(elevation, azimuth), v);
    }


public:
    virtual void clear()
    {
        if (post!=0) {
            delete post;
        }
        post=0;
    }
    
    
    virtual WavGroupAzimuthElevation* getResult(bool p)
    {
        if (!post || !p) {
            return bands;
        }
        return post;
    }
        
    //
    //
    //
        
    virtual bool postFilter(int mode, int deg1, int deg2, int th)
    {
        try {
          PostFilterMode post1mode = (PostFilterMode)mode;
          if (post1mode==NO_POST) return true;
          if (bands==0) return false;

          WavGroupAzimuthElevation *src;
          bool srcDel=false;
          if (post!=0) {
              src=post;
              srcDel=true;
          } else {
              src=bands;
          }
          post = postFilterAzEl(*src, post1mode, deg1, deg2, th);
          if (srcDel)	{
              delete src;
          }
          return post!=0;
        }
        catch (std::runtime_error* e) {
            //LogFmtA("Error in postFilter %d: %s",mode,e->what());
            return false;
        }
    }


    /**
      *  @param frames1 samples to average over, whole file if  <0
      *  @param frames2 step to go through input file with
      */
    virtual bool postAverage(double frames1=-1, double frames2=0)
    {
        if (bands==0) return false;
        WavGroupAzimuthElevation *src;
        bool srcDel=false;
        if (post!=0) {
            src=post;		
            srcDel=true;
        } else {
            src=bands;
        }
        post = postFilterAverage(*src,frames1,frames2);
        if (srcDel)	{
            delete src;
        }
        return post!=0;
    }

    virtual void postBandSum(int n,int th) 
    {    
        postFilter(POST_SUMB,n,th,0);
    }

    //
    //
    //
    
    
    virtual bool peakFilter1(Audio::Wave&in, Audio::SparseWave&out, PostFilterMode mode, int deg1, int deg2,int th)
    {
        try {
          if (mode == POST_SUME_AZPOAP) {
              Audio::SparseWave tmp;
              elevationSum( in, tmp);
              peakAzimuth( tmp, out,  deg1, deg2, th);
              return true;
          }
          if (mode == POST_MAXE_AZPOAP) {
              Audio::SparseWave tmp;
              elevationMax( in, tmp);
              peakAzimuth( tmp, out,  deg1, deg2, th);
              return true;
          }
          if (mode == POST_MAXE_AZPOA) {
              Audio::SparseWave tmp;
              elevationMax(in, tmp);
              poaAzimuth( tmp, out,  deg1, deg2, th);
              return true;
          }
          if (mode==POST_AZPOAP) {
              peakAzimuth( in, out,  deg1, deg2, th);
              return true;
          }
          if (mode==POST_AZPOA) {
              poaAzimuth( in, out,  deg1, deg2, th);
              return true;
          }
          if (mode==POST_THRESHOLD) {
              threshold( in, out,  (double) deg1 / 32767.0);
              return true;
          }

	 
          if (mode==POST_AEGRID) {
              peakAzimuthElevationGrid( in, out, deg1, deg2, th);
              return true;
          }
          if (mode==POST_AVG) {
              averageAzimuth(in,out,deg1,th/32767.0);
              return true;
          }
          if (mode==POST_SUME) {
              elevationSum(in, out);
              return true;
          }
          if (mode==POST_MAXE) {
              elevationMax( in, out);
              return true;
          }
          if (mode==POST_SUME_ARGMAX) {
              Audio::SparseWave tmp;
              elevationSum( in, tmp);
              argmax( tmp, out);
              return true;
          }
          if (mode==POST_ARGMAX) {
              argmax( in, out);
              return true;
          }
        }
        catch (std::runtime_error* e) {
            qWarning("Error in peakFilter %d: %s",mode,e->what());
            return false;
        }
		return false;
    }
 
    virtual WavGroupAzimuthElevation* postFilterAzEl( Audio::WavGroup & data , PostFilterMode mode, int deg1, int deg2,int th) 
    {            
        if (mode == POST_SUMB) {
            WavGroupAzimuthElevation* ret = new WavGroupAzimuthElevation(backPro);
            Audio::SparseWave* p = new Audio::SparseWave();
            AzimuthElevationFilter::bandSum(data, *p, deg1, pow(2.0, (double)deg2/6.0));	 
            ret->append( QSharedPointer<Audio::Wave>(p) );
            return ret;
        }    
		if (mode == POST_THRES_BANDC) {
			AzimuthElevationFilter::thresholdBandcount(data,deg1, pow(2.0, (double)deg2/6.0));	
			WavGroupAzimuthElevation* ret = new WavGroupAzimuthElevation(backPro);
			ret->clone(data);
			return ret;
		}
        WavGroupAzimuthElevation* ret = new WavGroupAzimuthElevation(backPro);
        int bandCount = data.count();
        for ( int band=0;band<bandCount;band++ ) {
            Audio::SparseWave* p = new Audio::SparseWave();
            Audio::Wave* in = data.at(band).data();
            if (in) {
                if (!peakFilter1( *in, *p , mode,deg1,deg2,th)) {
                    delete ret;
                    return 0;
                }
            }
            ret->append( QSharedPointer<Audio::Wave>(p) );
        }
        return ret;
    }


    /**
      *  @param tavg  samples to average over, whole file if  <0
      *  @param tout  step to go through input file with
      */
    virtual WavGroupAzimuthElevation* postFilterAverage( Audio::WavGroup & data, double tavg, double tout=1.0)
    {    
        WavGroupAzimuthElevation* ret = new WavGroupAzimuthElevation(backPro);		         
        int bandCount = data.count();
        for ( int band=0;band<bandCount;band++ ) {
            Audio::Wave* wave = data.at(band).data();
//             LogFmtA("AzimuthElevationFilter.average fs = %f len = %f avg = %f out =  %f",
//                     wave->getSamplingFrequency(), wave->getTotalSeconds(), tavg, tout);
            Audio::SparseWave* p = new Audio::SparseWave();
            if (wave) {
                calcAverageSparse(*wave, *p, tavg, tout);
            }
            ret->append( QSharedPointer<Audio::Wave>(p) );
        }
        return ret;
    }
        
    void prepare(Audio::Wave& in, const char* operation="");

    static void bandSum( Audio::WavGroup& data, Audio::SparseWave & out, int minBands=1,double th=64.0/32767.0);
	
	static void thresholdBandcount( Audio::WavGroup& data, int minBands, double th);

    void elevationMax(Audio::Wave& in, Audio::SparseWave & out);

    void elevationSum(Audio::Wave& in, Audio::SparseWave & out);

    void argmax(Audio::Wave& in, Audio::SparseWave & out);

    void threshold( Audio::Wave& in, Audio::SparseWave & out,double th);

    void peakAzimuth( Audio::Wave& in, Audio::SparseWave & out, int average1, int average2, int postThreshold)
	{
        poaAzimuthSub(true, in, out, average1, average2, postThreshold);
	}

    void poaAzimuth( Audio::Wave& in, Audio::SparseWave & out, int average1, int average2, int postThreshold)
	{
        poaAzimuthSub(false,in, out, average1, average2, postThreshold);
	}

    void convolveWithGaussian(Audio::Wave& in, Audio::Wave & out);

    void poaAzimuthSub(bool poap, Audio::Wave& in, Audio::SparseWave & out,  int average1, int average2, int postThreshold);

    void peakAzimuthElevationGrid(Audio::Wave& in, Audio::SparseWave & out, int average1, int average2, int postThreshold);

    void averageAzimuth(Audio::Wave& in, Audio::SparseWave & out, int average2, double threshold=0.0);

};



